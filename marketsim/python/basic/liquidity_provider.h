#ifndef _marketsim_python_basic_liquidity_provider_h_included
#define _marketsim_python_basic_liquidity_provider_h_included

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/order_pool.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/agent/liquidity_provider.h>
#include <marketsim/agent/canceller.h>
#include <marketsim/agent/agent_for_order.h>

namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

   //--------------------------------------------------------------- Agents

   namespace agent 
   {
       using namespace marketsim::agent;

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
                class_<
                    LiquidityProviderT, 
                    boost::intrusive_ptr<LiquidityProviderT>,
                    boost::noncopyable
                > 
                c(name.c_str(), no_init);

                base::py_visit(c);

                c.def("sendOrder", &LiquidityProviderT::sendOrder);

                register_6<LiquidityProviderT, boost::intrusive_ptr<OrderBook>, 
                    py_object, py_object, py_object, py_object, Price>(c);
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
   }
}}
#endif
