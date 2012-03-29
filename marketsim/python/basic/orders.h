#ifndef _marketsim_python_basic_orders_h_included
#define _marketsim_python_basic_orders_h_included

#include <boost/python.hpp>

#include <marketsim/order/base.h>
#include <marketsim/order/link_to_agent.h>
#include <marketsim/order/in_pool.h>
#include <marketsim/order/cancellable.h>


namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

    //-----------------------------------------  Orders
    namespace order 
    {
        using namespace marketsim::order;

        template <Side SIDE>
            struct MarketT : 
                WithLinkToAgent <agent::IAgentForMarketOrder<MarketT<SIDE> >*,
                MarketOrderBase <SIDE, 
                derived_is      <
                MarketT         <SIDE> 
                > > >
            {
                MarketT(Volume v, agent::IAgentForMarketOrder<MarketT<SIDE> >* s) : base(boost::make_tuple(v,s)) {}
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
}}
#endif