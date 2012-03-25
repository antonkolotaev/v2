#ifndef _marketsim_test_orders_h_included_
#define _marketsim_test_orders_h_included_

#include <marketsim/order/base.h>
#include <marketsim/order/execution_history.h>

namespace marketsim {
namespace order     {

    template <Side SIDE>
        struct LimitOrderT : 
            ExecutionHistory<
            LimitOrderBase  <SIDE, 
            RefCounted      <
            derived_is      <
            LimitOrderT     <SIDE> 
            > > > >  
    {
        typedef 
            ExecutionHistory<
            LimitOrderBase  <SIDE, 
            RefCounted      <
            derived_is      <
            LimitOrderT     <SIDE> 
            > > > >  
            base;
                
        LimitOrderT(PriceVolume const & x) : base(x) {}

        void on_released()
        {
            delete this;
        }
    };

    typedef LimitOrderT<Buy>    LimitOrderBuy;
    typedef LimitOrderT<Sell>   LimitOrderSell;

    template <Side SIDE>
        struct MarketOrderT : 
                ExecutionHistory<
                MarketOrderBase <SIDE, 
                derived_is      <
                MarketOrderT    <SIDE> 
                > > >   
    {
        typedef 
            ExecutionHistory<
            MarketOrderBase <SIDE, 
            derived_is      <
            MarketOrderT    <SIDE> 
            > > >   
            base;
                    
        MarketOrderT(Price x) : base(x) {}
    };

    typedef MarketOrderT<Buy>   MarketOrderBuy;
    typedef MarketOrderT<Sell>  MarketOrderSell;
}}

#endif
