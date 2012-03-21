#include <boost/python.hpp>

//#define MARKETSIM_BOOST_PYTHON

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

#include <marketsim/history.h>

#include "with_history_in_deque.h"
#include "PnL_Quantity_history_in_deque.h"

namespace marketsim {
namespace fast {

    using namespace boost::python;

    namespace agent {
        template <Side SIDE> struct LiquidityProviderT;
    }

    //-----------------------------------------  Orders
    namespace order
    {
        using namespace  marketsim::order;

        template <Side SIDE, typename Sender>
        struct MarketT : 
            WithLinkToAgent<Sender,
            MarketOrderBase<SIDE, MarketT<SIDE, Sender> 
            > >
        {
            MarketT(Volume v, Sender s) : base(boost::make_tuple(v,s)) {}
        };

        template <Side SIDE>
        struct LimitT : 
            WithCancelPosition  <
            WithLinkToAgent     < agent::LiquidityProviderT<SIDE>*,
            InPool              < PlacedInPool, 
            LimitOrderBase      < SIDE, 
            derived_is          <
            LimitT              < SIDE
            > > > > > >
        {
            LimitT(PriceVolume const &x, object_pool<LimitT> * h, agent::LiquidityProviderT<SIDE> * a) 
                :   base(boost::make_tuple(boost::make_tuple(x, h), a))
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
        struct with_history:   
                WithHistoryInDeque  <
                    OrderQueue      <   boost::intrusive_ptr<order::LimitT<SIDE> >,
                    with_history    < SIDE
                    > > >
        {
            with_history() {}

            static std::string py_name() 
            {
                return "OrderQueue_" + marketsim::py_name<side_tag<SIDE> >();
            }

            static void py_register(std::string const & name = py_name())
            {
                class_<with_history, boost::noncopyable> c(name.c_str());

                base::py_visit(c);
            }
        private:
            with_history(with_history const &);
        };
   }

   struct OrderBook 
       : marketsim::OrderBook<order_queue::with_history<Buy>, order_queue::with_history<Sell> >    
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
        struct LiquidityProviderT :
            LiquidityProvider   < rng::exponential<Time>, rng::normal<PriceF>, rng::exponential<VolumeF>, 
            OrderCanceller      < rng::exponential<Time>, boost::intrusive_ptr<order::LimitT<SIDE> >, 
            PnL_Quantity_History_InDeque <
            LinkToOrderBook     < OrderBook*, 
            PrivateOrderPool    < order::LimitT<SIDE>, 
            AgentBase           < LiquidityProviderT<SIDE> 
            > > > > > > 
        {
		    LiquidityProviderT( OrderBook * book, 
                    Time        meanCreationTime, 
                    Time        meanCancellationTime, 
                    PriceF      priceSigma,
                    VolumeF     meanOrderVolume, 
                    Price       initialPrice) 
                : base(
                    boost::make_tuple(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            rng::exponential<Time>(meanCancellationTime)),
                        rng::exponential<Time>(meanCreationTime), 
                        rng::normal<PriceF>(0., priceSigma), 
                        meanOrderVolume, 
                        initialPrice
                    ))
            {}

            static std::string py_name() 
            {
                return "LiquidityProvider_" + marketsim::py_name<side_tag>();
            }

            static void py_register(std::string const &name = py_name())
            {
                class_<LiquidityProviderT, boost::noncopyable> c(name.c_str(), init<OrderBook*, Time, Time, PriceF, VolumeF, Price>());

                base::py_visit(c);

                c.def("sendOrder", &LiquidityProviderT::sendOrder);
            }


            void sendOrder(Price p, Volume v)
            {
                order::LimitT<SIDE> * o= base::createOrder(pv(p,v));

                base::processOrder(o);
            }
        };

        struct FV_Trader :
            PnL_Quantity_History_InDeque<
            FundamentalValueTrader  <rng::exponential<Time>, rng::exponential<VolumeF>, 
            MarketOrderFactory      <order::MarketT<Buy, FV_Trader*>, order::MarketT<Sell, FV_Trader*>, 
            LinkToOrderBook         <OrderBook*, 
            AgentBase               <FV_Trader
            > > > > >
        {
            FV_Trader(OrderBook *book, Price FV, Time creationTime, VolumeF meanVolume)
                :   base(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            rng::exponential<Time>(creationTime), 
                            rng::exponential<VolumeF>(meanVolume), 
                            FV)
                        )
            {}

            static void py_register(const char * name)
            {
                class_<FV_Trader, boost::noncopyable> c(name, init<OrderBook*, Price, Time, VolumeF>());
                base::py_visit(c);
            }
        };

        struct Signal_Trader :
            PnL_Quantity_History_InDeque<
            SignalTrader        <rng::exponential<VolumeF>, 
            MarketOrderFactory  <order::MarketT<Buy, Signal_Trader*>, order::MarketT<Sell, Signal_Trader*>, 
            LinkToOrderBook     <OrderBook*, 
            AgentBase           <Signal_Trader> 
            > > > >
        {
            Signal_Trader(OrderBook * book, VolumeF meanVolume, double threshold)
                :   base(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            rng::exponential<VolumeF>(meanVolume), 
                            threshold
                        ))
            {}

            static void py_register(const char * name)
            {
                class_<Signal_Trader, boost::noncopyable> c(name, init<OrderBook*, VolumeF, double>());
                base::py_visit(c);
            }
        };

        struct Signal : marketsim::agent::Signal<rng::exponential<Time>, rng::normal<double>, Signal_Trader*>
        {
            Signal(Signal_Trader * trader, Time meanUpdateTime, double deltaSigma)
                :   base(rng::exponential<Time>(meanUpdateTime), rng::normal<double>(0., deltaSigma), trader)
            {}

            static void py_register(const char * name)
            {
                class_<Signal, boost::noncopyable> c(name, init<Signal_Trader*, Time, double>());
                base::py_visit(c);
            }
        };

        struct Noise_Trader :
            PnL_Quantity_History_InDeque<
            NoiseTrader         < rng::exponential<Time>,     rng::exponential<VolumeF>,
            MarketOrderFactory  < order::MarketT<Buy,Noise_Trader*>, order::MarketT<Sell,Noise_Trader*>, 
            LinkToOrderBook     < OrderBook*, 
            AgentBase           < Noise_Trader
            > > > > >
        {
            Noise_Trader(OrderBook * book, Time meanInterval, VolumeF meanVolume)
                :   base(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            rng::exponential<Time>(meanInterval), 
                            rng::exponential<VolumeF>(meanVolume)
                        ))
            {}

            static void py_register(const char * name)
            {
                class_<Noise_Trader, boost::noncopyable> c(name, init<OrderBook*, Time, VolumeF>());
                base::py_visit(c);
            }
        };
    }
}}

template <typename T>
    void py_register(const char * name)
    {
        static bool registered = false;
        if (!registered) {
            T::py_register(name);
            registered = true;
        }
    }

template <typename T>
    void py_register()
    {
        static bool registered = false;
        if (!registered) {
            T::py_register();
            registered = true;
        }
    }

BOOST_PYTHON_MODULE(fast)
{
	using namespace marketsim::fast;

    py_register<marketsim::Scheduler>("Scheduler");

	using marketsim::Sell;
	using marketsim::Buy;

    py_register<order_queue::with_history<Buy> >();
    py_register<order_queue::with_history<Sell> >();

    py_register<OrderBook>();

    py_register<agent::LiquidityProviderT<Sell> >();
    py_register<agent::LiquidityProviderT<Buy> >();

    py_register<agent::FV_Trader>("FV_Trader");
    py_register<agent::Signal_Trader>("Signal_Trader");
    py_register<agent::Signal>("Signal");
    py_register<agent::Noise_Trader>("Noise_Trader");
}

