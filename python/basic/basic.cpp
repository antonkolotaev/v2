#include <boost/python.hpp>
#include <list>
#include <set>
//#define MARKETSIM_BOOST_PYTHON

#include <marketsim/py_ref_counted.h>

#include <marketsim/object_pool.h>
#include <marketsim/order_book.h>

#include <marketsim/order/base.h>
#include <marketsim/order/link_to_agent.h>
#include <marketsim/order/in_pool.h>
#include <marketsim/order/cancellable.h>

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/order_pool.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/agent/liquidity_provider.h>
#include <marketsim/agent/canceller.h>
#include <marketsim/agent/fundamental_value_trader.h>
#include <marketsim/agent/market_order_factory.h>
#include <marketsim/agent/signal.h>
#include <marketsim/agent/signal_trader.h>
#include <marketsim/agent/noise_trader.h>
#include <marketsim/agent/agent_for_order.h>

#include <marketsim/history.h>

#include "with_history_in_deque.h"
#include "PnL_Quantity_history_in_deque.h"


namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

    template <class T>
    struct py_value
    {
        py_value(py_object value) : value(value) {}

        T operator () ()
        {
            return boost::python::extract<T>(value());
        }

        py_object value;
    };

    struct py_callback
    {   
        py_callback(py_object c) 
        {
            callbacks.push_back(c);
        }

        py_callback(Dummy) {}       
        py_callback() {}

        template <class T>
            void operator () (T * x)
            {
                reference_existing_object::apply<T*>::type converter;
                PyObject* obj = converter( x );
                object real_obj = object( handle<>( obj ) );

                BOOST_FOREACH(py_object callback, callbacks)
                {
                    if (!callback.is_none())
                    {
                        callback(real_obj);
                    }
                }
            }

        void add(py_object x)
        {
            callbacks.push_back(x);
        }

        void remove(py_object x)
        {
            callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), x), callbacks.end());
        }


        static void py_register()
        {
            class_<py_callback>("py_callback")
                .def("add", &py_callback::add)
                .def("remove", &py_callback::remove)
                ;
        }

        std::list<py_object>   callbacks;
    };

    //-----------------------------------------  Orders
    namespace order 
    {
        using namespace marketsim::order;

        template <Side SIDE, typename Sender>
            struct MarketT : 
                WithLinkToAgent <Sender,
                MarketOrderBase <SIDE, 
                derived_is      <
                MarketT         <SIDE, Sender> 
                > > >
            {
                MarketT(Volume v, Sender s) : base(boost::make_tuple(v,s)) {}
            };

        template <Side SIDE>
            struct LimitT : 
                    WithCancelPosition  <
                    WithLinkToAgent     < agent::IAgentForOrder<LimitT<SIDE> >*,
                    InPool              < PlacedInPool, 
                    LimitOrderBase      < SIDE, 
                    RefCounted          <
                    derived_is          <
                    LimitT              < SIDE
                > > > > > > >
            {
                LimitT(PriceVolume const &x, object_pool<LimitT> * h, agent::IAgentForOrder<LimitT<SIDE> > * ag) 
                    :   base(boost::make_tuple(boost::make_tuple(x, h), ag))
                {}

                // we don't export LimitT to Python since it seems to be not useful for fast simulation
            };
    }
            
    typedef order::LimitT<Buy>     LimitBuy;
    typedef order::LimitT<Sell>    LimitSell;

   typedef boost::intrusive_ptr<LimitBuy>   LimitBuyPtr;
   typedef boost::intrusive_ptr<LimitSell>  LimitSellPtr;

   //---------------------------------------------- OrderBook

   namespace order_queue
   {
       using namespace marketsim::order_queue;
       template <Side SIDE>
            struct with_callback  :   
                    OnQueueTopChanged   < py_callback,
                    OrderQueue          < boost::intrusive_ptr<order::LimitT<SIDE> >, 
                    with_callback       < SIDE
                    > > > 
            {
                with_callback() {}

                static std::string py_name() 
                {
                    return "OrderQueue_" + marketsim::py_name<side_tag<SIDE> >();
                }

                static void py_register(std::string const & name = py_name())
                {
                    class_<with_callback, boost::noncopyable> c(name.c_str());

                    base::py_visit(c);
                }
            private:
                with_callback(with_callback const &);
            };
   }

   struct OrderBook : 
       marketsim::OrderBook < order_queue::with_callback<Buy>, order_queue::with_callback<Sell>, 
       PyRefCounted         < IRefCounted >
       >    
   {
       OrderBook() {}

       static void py_register()
       {
           class_<OrderBook, boost::noncopyable> c("OrderBook");

           base::py_visit(c);
       }

   private:
       OrderBook(OrderBook const &);
   };

   //--------------------------------------------------------------- Agents

   namespace agent 
   {
       using namespace marketsim::agent;

       template <Side SIDE>
        struct LimitOrderTraderT :
            IAgentForOrderImpl      < order::LimitT<SIDE>,
            OnPartiallyFilled       < py_callback, 
            OrdersSubmittedInVector < boost::intrusive_ptr<order::LimitT<SIDE> >, 
            PnL_Holder              <
            Quantity_Holder         <
            LinkToOrderBook         < boost::intrusive_ptr<OrderBook>, 
            SharedOrderPool         < order::LimitT<SIDE>, 
            AgentBase               < LimitOrderTraderT<SIDE> 
            > > > > > > > >
        {
            LimitOrderTraderT(boost::intrusive_ptr<OrderBook> book) 
                : base(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            dummy))
            {}

            static std::string py_name() 
            {
                return "LimitOrderTrader_" + marketsim::py_name<side_tag<SIDE> >();
            }

            static void py_register()
            {
                class_<LimitOrderTraderT, boost::noncopyable> c(py_name().c_str(), init<OrderBook*>());

                base::py_visit(c);

                c.def("sendOrder", &LimitOrderTraderT::sendOrder);
            }

            void sendOrder(Price p, Volume v)
            {
                if (v > 0)
                {
                    order::LimitT<SIDE> * order = base::createOrder(pv(p,v));

                    base::processOrder(order);
                }
            }
        };


       template <Side SIDE>
        struct LiquidityProviderT :
            IAgentForOrderImpl  < order::LimitT<SIDE>,
            LiquidityProvider   < py_value<Time>, py_value<PriceF>, py_value<VolumeF>, 
            OrderCanceller      < py_value<Time>, boost::intrusive_ptr<order::LimitT<SIDE> >, 
            PnL_Quantity_History_InDeque <
            LinkToOrderBook     < boost::intrusive_ptr<OrderBook>, 
            SharedOrderPool     < order::LimitT<SIDE>, 
            PyRefCounted        <
            AgentBase           < LiquidityProviderT<SIDE>, IRefCounted
            > > > > > > > >
        {
            LiquidityProviderT( boost::intrusive_ptr<OrderBook> book, 
                    py_object   creationTimeDistr, 
                    py_object   cancelTimeDistr, 
                    py_object   priceDistr,
                    py_object   volumeDistr, 
                    Price       initialPrice) 
                : base(
                    boost::make_tuple(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            cancelTimeDistr),
                        creationTimeDistr, 
                        priceDistr, 
                        volumeDistr, 
                        initialPrice
                    ))
            {}

            static std::string py_name() 
            {
                return "LiquidityProvider_" + marketsim::py_name<side_tag>();
            }

            static void py_register(std::string const &name = py_name())
            {
                class_<LiquidityProviderT, boost::noncopyable> c(name.c_str(), init<OrderBook*, py_object, py_object, py_object, py_object, Price>());

                base::py_visit(c);

                c.def("sendOrder", &LiquidityProviderT::sendOrder);
            }


            void sendOrder(Price p, Volume v)
            {
                if (v > 0)
                {
                    order::LimitT<SIDE> * o = base::createOrder(pv(p,v));

                    base::processOrder(o);
                }
            }
        };

        struct PnL_Quantity_History_Collector 
            :   history::Collector<Get_PnL_Quantity, history::InDeque<PriceVolume> >
        {
            
        };

        struct MarketOrderTrader :
            OnPartiallyFilled       < py_callback,
            PnL_Holder              <
            Quantity_Holder         <
            MarketOrderFactory      < order::MarketT<Buy, MarketOrderTrader*>, order::MarketT<Sell, MarketOrderTrader*>, 
            LinkToOrderBook         < OrderBook*, 
            AgentBase               < MarketOrderTrader
            > > > > > >
        {
            MarketOrderTrader(OrderBook * book)
                :   base(boost::make_tuple(boost::make_tuple(dummy, book), dummy))
            {}

            static void py_register()
            {
                class_<MarketOrderTrader, boost::noncopyable > c("MarketOrderTrader", init<OrderBook*>());
                base::py_visit(c);
            }
        };

        struct FV_Trader :
            OnPartiallyFilled       < py_callback,
            PnL_Quantity_History_InDeque<
            FundamentalValueTrader  < py_value<Time>, py_value<VolumeF>, 
            MarketOrderFactory      < order::MarketT<Buy, FV_Trader*>, order::MarketT<Sell, FV_Trader*>, 
            LinkToOrderBook         < OrderBook*, 
            PyRefCounted            <
            AgentBase               < FV_Trader, IRefCounted
            > > > > > > >
        {
            FV_Trader(OrderBook *book, Price FV, py_object intervalDist, py_object volumeDist)
                :   base(
                        boost::make_tuple(
                            boost::make_tuple(
                                boost::make_tuple(dummy, book), 
                                intervalDist, 
                                volumeDist, 
                                FV),
                            dummy)
                        )
            {}

            static void py_register(const char * name)
            {
                class_<FV_Trader, boost::noncopyable > c(name, init<OrderBook*, Price, py_object, py_object>());
                base::py_visit(c);
            }
        };

        struct Signal_Trader :
            PnL_Quantity_History_InDeque<
            SignalTrader                < py_value<VolumeF>, 
            MarketOrderFactory          < order::MarketT<Buy, Signal_Trader*>, order::MarketT<Sell, Signal_Trader*>, 
            LinkToOrderBook             < OrderBook*, 
            AgentBase                   < Signal_Trader> 
            > > > >
        {
            Signal_Trader(OrderBook * book, py_object volumeDist, double threshold)
                :   base(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            volumeDist, 
                            threshold
                        ))
            {}

            static void py_register(const char * name)
            {
                class_<Signal_Trader, boost::noncopyable> c(name, init<OrderBook*, py_object, double>());
                base::py_visit(c);
            }
        };

        struct Signal : 
            marketsim::agent::Signal < py_value<Time>, py_value<double>, Signal_Trader*, 
            PyRefCounted             < IRefCounted >
            >
        {
            Signal(Signal_Trader * trader, py_object updateDist, py_object signalDist)
                :   base(updateDist, signalDist, trader)
            {}

            static void py_register(const char * name)
            {
                class_<Signal, boost::noncopyable> c(name, init<Signal_Trader*, py_object, py_object>());
                base::py_visit(c);
            }
        };

        struct Noise_Trader :
            PnL_Quantity_History_InDeque<
            NoiseTrader         < py_value<Time>, py_value<VolumeF>,
            MarketOrderFactory  < order::MarketT<Buy,Noise_Trader*>, order::MarketT<Sell,Noise_Trader*>, 
            LinkToOrderBook     < OrderBook*, 
            PyRefCounted        <
            AgentBase           < Noise_Trader, IRefCounted
            > > > > > >
        {
            Noise_Trader(OrderBook * book, py_object interval, py_object meanVolume)
                :   base(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            interval, 
                            meanVolume
                        ))
            {}

            static void py_register(const char * name)
            {
                class_<Noise_Trader, boost::noncopyable> c(name, init<OrderBook*, py_object, py_object>());
                base::py_visit(c);
            }
        };

   }

    struct ScheduledEvent : PyRefCounted<EventHandlerBase>
    {
        ScheduledEvent(TimeInterval dt, boost::python::object handler, boost::python::object callback) 
            : handler(handler), callback(callback)
        {
            schedule(dt);
        }

        void process()
        {
            handler();
        }

        void on_released()
        {
            callback();
            // do nothing since it is supposed to be handled by the Python runtime
            // NB! It is very important to hold a reference to this object in python 
            // TBD: implement boost::intrusive_ptr support Boost.Python
        }

        ~ScheduledEvent()
        {
            int a = 1;
        }

        boost::python::object  handler, callback;
        
        static void py_register()
        {
            using namespace boost::python;
            class_<ScheduledEvent, boost::noncopyable>("Event", init<TimeInterval, object, object>())
                .def("cancel", &ScheduledEvent::cancel)
                ;
        }
    };

    struct Timer : PyRefCounted<EventHandlerBase>
    {
        Timer(py_object intervals, py_object handler, py_object callback)
            :   handler  (handler)
            ,   intervals(intervals)
            ,   callback (callback)
        {
            schedule(this->intervals());
        }

        void process()
        {
            handler();
            schedule(intervals()); 
        }

        void on_released()
        {
            callback();
            // do nothing since the object is to be managed by Python run time
        }

        ~Timer()
        {
            // in order to kill a timer cancel() method is to be called
        }

        static void py_register()
        {
            using namespace boost::python;

            class_<Timer, boost::noncopyable>("Timer", init<object, object, object>())
                .def("cancel", &Timer::cancel)
                ;
        }
        

        py_object               handler, callback;
        py_value<TimeInterval>  intervals;
    };

}}


BOOST_PYTHON_MODULE(basic)
{
	using namespace marketsim::basic;
    using marketsim::py_register;

    py_register<marketsim::Scheduler>("Scheduler");
    py_register<marketsim::PriceVolume>();

	using marketsim::Sell;
	using marketsim::Buy;

    py_register<order_queue::with_callback<Buy> >();
    py_register<order_queue::with_callback<Sell> >();

    py_register<OrderBook>();

    py_register<agent::LiquidityProviderT<Sell> >();
    py_register<agent::LiquidityProviderT<Buy> >();

    py_register<agent::LimitOrderTraderT<Sell> >();
    py_register<agent::LimitOrderTraderT<Buy> >();

    py_register<agent::FV_Trader>("FV_Trader");
    py_register<agent::Signal_Trader>("Signal_Trader");
    py_register<agent::Signal>("Signal");
    py_register<agent::Noise_Trader>("Noise_Trader");

    py_register<agent::MarketOrderTrader>();

    py_register<marketsim::rng::exponential<> >();
    py_register<marketsim::rng::constant<> >();
    py_register<marketsim::rng::gamma<> >();
    py_register<marketsim::rng::lognormal<> >();
    py_register<marketsim::rng::normal<> >();
    py_register<marketsim::rng::uniform_01<> >();
    py_register<marketsim::rng::uniform_real<> >();
    py_register<marketsim::rng::uniform_smallint<> >();

    py_register<py_callback>();

    py_register<marketsim::history::CollectInDeque<marketsim::py_PnL_Quantity> >();
    py_register<marketsim::history::CollectInDeque<marketsim::order_queue::py_BestPriceAndVolume> >();

    py_register<ScheduledEvent>();
    py_register<Timer>();
}

