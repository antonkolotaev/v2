#ifndef _marketsim_test_orders_h_included_
#define _marketsim_test_orders_h_included_

#include <marketsim/order/base.h>
#include <marketsim/order/filling.h>

namespace marketsim
{
    template <Side SIDE>
        struct LimitOrderT : 
            ExecutionHistory<
                LimitOrderBase<SIDE, 
                    LimitOrderT<SIDE> > >    
    {
        LimitOrderT(PriceVolume const & x) : base(x) {}
    };

    typedef LimitOrderT<Buy>    LimitOrderBuy;
    typedef LimitOrderT<Sell>   LimitOrderSell;

    template <Side SIDE>
        struct MarketOrderT : 
            ExecutionHistory<
                MarketOrderBase<SIDE, 
                    MarketOrderT<SIDE> > >    
    {
        MarketOrderT(Price x) : base(x) {}
    };

    typedef MarketOrderT<Buy>   MarketOrderBuy;
    typedef MarketOrderT<Sell>  MarketOrderSell;
}

#endif