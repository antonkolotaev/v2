#ifndef _marketsim_python_basic_limit_order_trader_h_included
#define _marketsim_python_basic_limit_order_trader_h_included

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/order_pool.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/agent/canceller.h>
#include <marketsim/agent/agent_for_order.h>
#include <marketsim/python/basic/order_book.h>

namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

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
            SharedOrderPool         < boost::intrusive_ptr<order::LimitT<SIDE> >, 
            PyRefCounted            <
            AgentBase               < LimitOrderTraderT<SIDE>, IRefCounted
            > > > > > > > > >
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
                class_<
                    LimitOrderTraderT, 
                    boost::intrusive_ptr<LimitOrderTraderT>,
                    boost::noncopyable
                > 
                c(py_name().c_str(), no_init);

                base::py_visit(c);

                c.def("sendOrder", &LimitOrderTraderT::sendOrder);

                marketsim::register_1<LimitOrderTraderT, boost::intrusive_ptr<OrderBook> >(c);
            }

            void sendOrder(Price p, Volume v)
            {
                if (v > 0)
                {
                    boost::intrusive_ptr<order::LimitT<SIDE> > order = base::createOrder(pv(p,v));

                    base::processOrder(order);
                }
            }
        };
   }
}}


#endif