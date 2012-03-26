#ifndef _marketsim_fast_market_order_h_included
#define _marketsim_fast_market_order_h_included

#include <marketsim/order/market_order_base.h>
#include <marketsim/order/link_to_agent.h>

namespace marketsim {
namespace fast {

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
    }
}}

#endif