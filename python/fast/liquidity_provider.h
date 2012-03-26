#ifndef _marketsim_fast_liquidity_provider_h_included
#define _marketsim_fast_liquidity_provider_h_included

#include <marketsim/py_ref_counted.h>
#include "order_book.h"
#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/order_pool.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/agent/liquidity_provider.h>
#include <marketsim/agent/canceller.h>
#include "PnL_Quantity_history_in_deque.h"

namespace marketsim {
namespace fast {

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
            AgentBase           < LiquidityProviderT<SIDE>, IRefCounted 
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
                    bases<IRefCounted>, 
                    boost::noncopyable
                > 
                c(name.c_str(), no_init);

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

    }
}}

MARKETSIM_PY_REGISTER(marketsim::fast::agent::LiquidityProviderT<marketsim::Sell>);
MARKETSIM_PY_REGISTER(marketsim::fast::agent::LiquidityProviderT<marketsim::Buy>);

#endif