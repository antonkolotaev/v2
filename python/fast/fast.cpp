#define BOOST_PYTHON_TRACE_REGISTRY
//#define MARKETSIM_BOOST_PYTHON
#include <boost/python.hpp>
#include <marketsim/ref_counted.h>
#include <marketsim/py_ref_counted.h>

#include <string>

//using namespace std;
//using namespace boost;
//using namespace boost::python;

typedef marketsim::IRefCounted IPyRefCounted;
namespace py = boost::python;
//------------------------------------------------------------------------------
class C;
typedef boost::intrusive_ptr<C> CPtr;

class C : public marketsim::PyRefCounted<IPyRefCounted>
{
public:
    C(std::string const &name) : 
      name(name)
      {
      }

      virtual ~C() 
      {
          std::cout << "~C('" << name << "')" << std::endl;
      }

      void set_child(CPtr child)
      {
          this->child = child;
      }

      CPtr get_child()
      {
          return child;
      }

      std::string get_name()
      {
          return name;
      }

      virtual std::string info()
      {
          std::string s =  "C['" + name + "']";
          if (child)
          {
              s += "-->" + child->info();
          }
          return s;
      }

private:
    std::string name;
    CPtr child;
};

//------------------------------------------------------------------------------
class CWrap : public marketsim::PyRefCountedPersistent<C>
{
public:
    CWrap(const std::string &name, bool persistent = false) : 
      marketsim::PyRefCountedPersistent<C>(boost::make_tuple(name, persistent))
      {
      }

      CWrap(PyObject * py_obj, const std::string &name, bool persistent = false) : 
      marketsim::PyRefCountedPersistent<C>(boost::make_tuple(name, persistent))
      {
          set_pyobject(py_obj);
      }

      std::string info()
      {
          if (get_pyobject())
          {
              return py::call_method<std::string>(get_pyobject(), 
                  "info"); 
          }
          else
          {
              return C::info();
          }
      }

      std::string default_info() 
      { 
          std::string s =  "CWrap['" + get_name() + "']";
          CPtr child = get_child();
          if (child)
          {
              s += "-->" + child->info();
          }
          return s;
      }
};

//------------------------------------------------------------------------------
std::string info(CPtr c)
{
    return c->info();
}

//------------------------------------------------------------------------------
CPtr new_C(std::string name)
{
    return CPtr(new C(name));
}

py::object init_C(py::object py_obj, std::string name)
{
    py::object return_value = py::call_method<py::object>(py_obj.ptr(), "__cons__", name);
    intrusive_ptr_set_pyobject(py::extract<C*>(py_obj), py_obj.ptr());
    return return_value;
}

#define MARKETSIM_HAS_REFERENCE_TO_PYTHON(X)\
namespace boost { namespace python  \
{                                   \
    template <>                     \
    struct has_back_reference<X>    \
        : mpl::true_                \
    {};                             \
}}

MARKETSIM_HAS_REFERENCE_TO_PYTHON(C);
MARKETSIM_HAS_REFERENCE_TO_PYTHON(CWrap);

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


MARKETSIM_HAS_REFERENCE_TO_PYTHON(marketsim::Scheduler);

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
            WithLinkToAgent     < weak_intrusive_ptr<agent::LiquidityProviderT<SIDE> >,
            InPool              < PlacedInPool, 
            LimitOrderBase      < SIDE, 
            RefCounted          <
            derived_is          <
            LimitT              < SIDE
            > > > > > > >
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

   struct OrderBook : 
       marketsim::OrderBook     < order_queue::with_history<Buy>, order_queue::with_history<Sell>,
       marketsim::PyRefCounted  < IRefCounted >
       >    
   {
       OrderBook() {}

       static void py_register()
       {
           class_<OrderBook, boost::intrusive_ptr<OrderBook>, bases<IPyRefCounted>, boost::noncopyable> c("OrderBook", no_init);

           base::py_visit(c);

           register_0<OrderBook>(c);
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
            LinkToOrderBook     < boost::intrusive_ptr<OrderBook>, 
            SharedOrderPool     < order::LimitT<SIDE>, 
            PyRefCounted        <
            HasWeakReferences   <
            AgentBase           < LiquidityProviderT<SIDE>, IPyRefCounted 
            > > > > > > > >
        {
		    LiquidityProviderT(
                    boost::intrusive_ptr<OrderBook> book, 
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
                class_<
                    LiquidityProviderT, 
                    boost::intrusive_ptr<LiquidityProviderT>, 
                    bases<IPyRefCounted>, 
                    boost::noncopyable
                > 
                c(name.c_str(), /*init<OrderBook*, Time, Time, PriceF, VolumeF, Price>()*/no_init);

                base::py_visit(c);

                c.def("sendOrder", &LiquidityProviderT::sendOrder);

                register_6<LiquidityProviderT, boost::intrusive_ptr<OrderBook>, Time, Time, PriceF, VolumeF, Price>(c);
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
            PyRefCounted            <
            AgentBase               <FV_Trader
            > > > > > >
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
            PyRefCounted        <
            AgentBase           <Signal_Trader> 
            > > > > >
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

        struct Signal : 
            marketsim::agent::Signal<rng::exponential<Time>, rng::normal<double>, Signal_Trader*,
            PyRefCounted            <
            derived_is              <Signal>
            > >
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
            PyRefCounted        <
            AgentBase           < Noise_Trader
            > > > > > >
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

MARKETSIM_HAS_REFERENCE_TO_PYTHON(marketsim::fast::OrderBook);
MARKETSIM_HAS_REFERENCE_TO_PYTHON(marketsim::fast::agent::LiquidityProviderT<marketsim::Sell>);
MARKETSIM_HAS_REFERENCE_TO_PYTHON(marketsim::fast::agent::LiquidityProviderT<marketsim::Buy>);


BOOST_PYTHON_MODULE(fast)
{
    using marketsim::py_register;

    py::class_<IPyRefCounted, boost::intrusive_ptr<IPyRefCounted>, boost::noncopyable>
        ("RefCounted", py::no_init)
        ;
    
    py::class_<C, boost::intrusive_ptr<C>, py::bases<IPyRefCounted>, boost::noncopyable>
        ("C", py::no_init)
        .def("__init__", init_C)
        .def("__cons__", py::make_constructor(new_C))
        .def("__del__",  &boost::intrusive_ptr_clear_pyobject)
        .def("info", &C::info)
        .add_property("child", &C::get_child, &C::set_child)
        .add_property("name", &C::get_name)
        ;

    py::class_<CWrap, boost::intrusive_ptr<CWrap>, py::bases<C>, boost::noncopyable >
        ("CWrap",py::init<const std::string&, bool>((py::arg("name"), 
        py::arg("persistent")=false)))
        .def("info", &C::info, &CWrap::default_info)
        ;

    py::def("new_C", new_C);
    py::def("info", info);

    // I haven't needed this yet, but...
    //implicitly_convertible<intrusive_ptr<CWrap>,intrusive_ptr<C> >();

    py::converter::intrusive_ptr_from_python<CWrap>();
    py::register_ptr_to_python< boost::intrusive_ptr<CWrap> >();

    py::converter::intrusive_ptr_from_python<C>();
    py::register_ptr_to_python< boost::intrusive_ptr<C> >();

    
    using namespace marketsim::fast;
    using marketsim::Scheduler;
    py_register<marketsim::Scheduler>("Scheduler");

    using marketsim::Sell;
    using marketsim::Buy;

    //py_register<order_queue::with_history<Buy> >();
    //py_register<order_queue::with_history<Sell> >();

    py_register<OrderBook>();

    py_register<agent::LiquidityProviderT<Sell> >();
    py_register<agent::LiquidityProviderT<Buy> >();

    py_register<agent::FV_Trader>("FV_Trader");
    py_register<agent::Signal_Trader>("Signal_Trader");
    py_register<agent::Signal>("Signal");
    py_register<agent::Noise_Trader>("Noise_Trader"); 
}
