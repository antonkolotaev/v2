
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

    template <Side SIDE>
       struct LimitT : 
            InPool<PlacedInPool, 
                LimitOrderBase<SIDE, 
                    LimitT<SIDE> > > 
    {
        template <typename X>
            LimitT(PriceVolume x, object_pool<LimitT> * h, X) 
                :   base(boost::make_tuple(x, h))
        {}
    };

   typedef LimitT<Buy>     LimitBuy;
   typedef LimitT<Sell>    LimitSell;

   typedef boost::intrusive_ptr<LimitBuy>   LimitBuyPtr;
   typedef boost::intrusive_ptr<LimitSell>  LimitSellPtr;

   typedef OrderBook<OrderQueue<LimitBuyPtr>, OrderQueue<LimitSellPtr> >    OrderBook;


   // !!! TODO: Extract object_pool<Order> to PrivatePool<> and SharedPool<>
   // it will also allow to use smart pointers to store orders and thus reduce potential errors
   template <Side SIDE>
   struct LiquidityProviderT : 
       LiquidityProvider<rng::constant<Time>, rng::constant<Price>, rng::constant<Volume>,
            LinkToOrderBook<OrderBook*, 
                PrivateOrderPool<LimitT<SIDE>, 
                    AgentBase<LiquidityProviderT<SIDE> > >
            >
       >
   {
        LiquidityProviderT(OrderBook *book) 
            :  base(boost::make_tuple(boost::make_tuple(dummy, book),
                                     rng::constant<Time>(1.), 
                                     rng::constant<Price>(2), 
                                     rng::constant<Volume>(5), 
                                     100))
        {}

   };

   struct MarketBuy : MarketOrderBase<Buy, MarketBuy>
   {
       MarketBuy(Volume v) : base(v) {}
   };

   TEST_CASE("liquidity_provider", "An agent sending limit orders")
   {
       Scheduler                    scheduler;
       OrderBook                    orderBook(2);
       LiquidityProviderT<Sell>     trader(&orderBook);

       scheduler.workTill(3.5);

       REQUIRE(!orderBook.empty<Sell>());
       REQUIRE(orderBook.bestPrice<Sell>() == 102);

       orderBook.processOrder(MarketBuy(10));

       REQUIRE(orderBook.bestPrice<Sell>() == 104);
       scheduler.workTill(4.5);
       
       orderBook.processOrder(MarketBuy(5));
       REQUIRE(orderBook.bestPrice<Sell>() == 106);
   }
}}  
