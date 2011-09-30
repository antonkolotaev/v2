
#include "catch.hpp"

#include <marketsim/rng.h>
#include <marketsim/scheduler.h>
#include <marketsim/order_book.h>

#include <marketsim/order/base.h>
#include <marketsim/order/in_pool.h>
#include <marketsim/order/link_to_agent.h>
#include <marketsim/order/cancellable.h>

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/canceller.h>
#include <marketsim/agent/order_pool.h>

namespace marketsim {
namespace {

    template <Side SIDE> struct AgentT;

    template <Side SIDE>
        struct LimitT : 
            WithCancelPosition<
                WithLinkToAgent<AgentT<SIDE>*,
                    InPool<PlacedInPool, 
                        LimitOrderBase<SIDE, 
                            LimitT<SIDE> > > > >
        {
            LimitT(PriceVolume const &x, object_pool<LimitT> * h, AgentT<SIDE> * agent) 
                :   base(boost::make_tuple(boost::make_tuple(x, h), agent))
            {}
        };
            
   typedef LimitT<Buy>     LimitBuy;
   typedef LimitT<Sell>    LimitSell;

   typedef boost::intrusive_ptr<LimitBuy>   LimitBuyPtr;
   typedef boost::intrusive_ptr<LimitSell>  LimitSellPtr;

   typedef OrderBook<OrderQueue<LimitBuyPtr>, OrderQueue<LimitSellPtr> >    OrderBook;

   struct always_0
   {
       always_0(int,int) {}

       int operator () () const { return 0; }
   };

   template <Side SIDE>
    struct AgentT :
            OrderCanceller      < rng::Generator<Time>, boost::intrusive_ptr<LimitT<SIDE> >, 
            LinkToOrderBook     < OrderBook*, 
            PrivateOrderPool    < LimitT<SIDE>, 
            AgentBase           < AgentT<SIDE> > 
            > >, always_0>
    {
        AgentT(OrderBook * book) 
            : base(boost::make_tuple(boost::make_tuple(dummy, book), new rng::constant<Time, rng::IGenerator<Time> >(1.))) 
        {}

        LimitT<SIDE> * sendOrder(Price p, Volume v)
        {
            LimitT<SIDE> * order = base::createOrder(pv(p,v));

            base::processOrder(order);

            return order;
        }
    };

    struct MarketBuy : MarketOrderBase<Buy, MarketBuy>
    {
        MarketBuy(Volume v) : base(v) {}
    };

    TEST_CASE("order_canceller", "")
    {
        Scheduler       scheduler;
        OrderBook       book;
        AgentT<Sell>    agent(&book);

        {
            // 1. Testing empty order book
            scheduler.workTill(3.5);

            // 2. Testing cancellations
            LimitSellPtr L1 = agent.sendOrder(105, 5);
            LimitSellPtr L2 = agent.sendOrder(100, 5);
            LimitSellPtr L3 = agent.sendOrder(90, 5);

            REQUIRE(!book.empty<Sell>());
            REQUIRE(book.bestPrice<Sell>() == 90);

            scheduler.workTill(4.5);

            assert(!book.empty<Sell>());
            REQUIRE(!book.empty<Sell>());
            REQUIRE(book.bestPrice<Sell>() == 90);

            assert(L1->cancelled());
            REQUIRE(L1->cancelled());
            REQUIRE(!L2->cancelled());
            REQUIRE(!L3->cancelled());

            scheduler.workTill(5.5);

            REQUIRE(!book.empty<Sell>());
            REQUIRE(book.bestPrice<Sell>() == 100);
            REQUIRE(!L2->cancelled());
            REQUIRE(L3->cancelled());

            scheduler.workTill(6.5);

            REQUIRE(book.empty<Sell>());
            REQUIRE(L2->cancelled());

            // 3. Test a situation of order filling
            L1 = agent.sendOrder(90, 5);
            L2 = agent.sendOrder(105, 5);
            L3 = agent.sendOrder(100, 5);

            book.processOrder(MarketBuy(5));
            REQUIRE(!book.empty<Sell>());
            REQUIRE(book.bestPrice<Sell>() == 100);
            REQUIRE(L1->filled());

            // 4. Test of an external order cancellation
            L3->onCancelled();
            REQUIRE(!book.empty<Sell>());
            REQUIRE(book.bestPrice<Sell>() == 105);

            scheduler.workTill(7.5);
            REQUIRE(book.empty<Sell>());
            REQUIRE(L2->filled());
        }
    }
}}