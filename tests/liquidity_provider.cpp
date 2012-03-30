
#include "catch.hpp"

#include <vector>
#include <marketsim/scheduler.h>
#include <marketsim/rng.h>

#include <marketsim/order_book.h>

#include <marketsim/order/base.h>
#include <marketsim/order/in_pool.h>
#include <marketsim/order/link_to_agent.h>

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/liquidity_provider.h>
#include <marketsim/agent/order_pool.h>

namespace marketsim {
namespace {

    namespace order 
    {
        using namespace marketsim::order;

        template <Side SIDE>
           struct LimitT : 
                    InPool        <PlacedInPool, 
                    LimitOrderBase<SIDE,
                    RefCounted    <
                    derived_is    <
                    LimitT        <SIDE> 
                    > > > >
        {
            typedef 
                InPool        <PlacedInPool, 
                LimitOrderBase<SIDE,
                RefCounted    <  
                derived_is    <
                LimitT        <SIDE> 
                > > > >
                base;
                            
            template <typename X>
                LimitT(PriceVolume x, object_pool<LimitT> * h, X) 
                    :   base(boost::make_tuple(x, h))
            {}
        };
    }

    typedef order::LimitT<Buy>     LimitBuy;
    typedef order::LimitT<Sell>    LimitSell;

   typedef boost::intrusive_ptr<LimitBuy>   LimitBuyPtr;
   typedef boost::intrusive_ptr<LimitSell>  LimitSellPtr;

   typedef 
       OrderBook<
            order_queue::OrderQueue<LimitBuyPtr>, 
            order_queue::OrderQueue<LimitSellPtr> 
       >    
       OrderBook;

   namespace agent 
   {
       using namespace marketsim::agent;

       template <Side SIDE>
       struct LiquidityProviderT : 
                LiquidityProvider   < rng::constant<Time>, rng::constant<Price>, rng::constant<Volume>,
                LinkToOrderBook     < OrderBook*, 
                SharedOrderPool     < boost::intrusive_ptr<order::LimitT<SIDE> >, 
                RefCounted          <
                AgentBase           < LiquidityProviderT<SIDE> 
                > > > > >
       {
            typedef 
                LiquidityProvider   < rng::constant<Time>, rng::constant<Price>, rng::constant<Volume>,
                LinkToOrderBook     < OrderBook*, 
                SharedOrderPool     < boost::intrusive_ptr<order::LimitT<SIDE> >, 
                RefCounted          <
                AgentBase           < LiquidityProviderT<SIDE> 
                > > > > >
                base;
                            
            LiquidityProviderT(OrderBook *book) 
                :  base(boost::make_tuple(boost::make_tuple(dummy, book),
                                         rng::constant<Time>(1.), 
                                         rng::constant<Price>(2), 
                                         rng::constant<Volume>(5), 
                                         100))
            {}

            void on_released() {}
       };
   }

   struct MarketBuy 
       : order::MarketOrderBase<Buy, derived_is <MarketBuy> >
   {
       MarketBuy(Volume v) : base(v) {}
   };

   TEST_CASE("liquidity_provider", "An agent sending limit orders")
   {
       Scheduler                        scheduler;
       OrderBook                        orderBook(2);
       agent::LiquidityProviderT<Sell>  trader(&orderBook);

       scheduler.workTill(3.5);

       assert(!orderBook.empty<Sell>());
       assert(orderBook.bestPrice<Sell>() == 102);

       orderBook.processOrder(MarketBuy(10));

       assert(orderBook.bestPrice<Sell>() == 104);
       scheduler.workTill(4.5);
       
       orderBook.processOrder(MarketBuy(5));
       assert(orderBook.bestPrice<Sell>() == 106);
   }
}}  
