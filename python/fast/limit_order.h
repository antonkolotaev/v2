#ifndef _marketsim_limit_order_h_included
#define _marketsim_limit_order_h_included

#include <marketsim/order/limit_order_base.h>
#include <marketsim/ref_counted.h>
#include <marketsim/order/in_pool.h>
#include <marketsim/order/link_to_agent.h>
#include <marketsim/order/cancellable.h>

namespace marketsim {
namespace fast {

    namespace agent {
        template <Side SIDE> struct LiquidityProviderT;
    }

    namespace order
    {
        using namespace  marketsim::order;

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
}}
#endif