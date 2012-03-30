
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

    namespace agent {
        template <Side SIDE> struct AgentT;
    }
    namespace order {

        using namespace marketsim::order;

        template <Side SIDE>
            struct LimitT : 
                    WithCancelPosition  <
                    WithLinkToAgent     <weak_intrusive_ptr<agent::AgentT<SIDE> >,
                    InPool              <PlacedInPool, 
                    LimitOrderBase      <SIDE, 
                    RefCounted          <
                    derived_is          <
                    LimitT              <SIDE
                    > > > > > > >
            {
                typedef 
                    WithCancelPosition  <
                    WithLinkToAgent     <weak_intrusive_ptr<agent::AgentT<SIDE> >,
                    InPool              <PlacedInPool, 
                    LimitOrderBase      <SIDE, 
                    RefCounted          <
                    derived_is          <
                    LimitT              <SIDE
                    > > > > > > >
                     base; 
                                                 
                LimitT(PriceVolume const &x, object_pool<LimitT> * h, agent::AgentT<SIDE> * a) 
                    :   base(boost::make_tuple(boost::make_tuple(x, h), a))
                {}
            };

            struct MarketBuy 
                : MarketOrderBase<Buy, derived_is <MarketBuy> >
            {
                MarketBuy(Volume v) : base(v) {}
            };
    }
            
    typedef order::LimitT<Buy>     LimitBuy;
    typedef order::LimitT<Sell>    LimitSell;

   typedef boost::intrusive_ptr<LimitBuy>   LimitBuyPtr;
   typedef boost::intrusive_ptr<LimitSell>  LimitSellPtr;

   typedef OrderBook<order_queue::OrderQueue<LimitBuyPtr>, order_queue::OrderQueue<LimitSellPtr> >    OrderBook;

   struct always_0
   {
       always_0(int,int) {}

       int operator () () const { return 0; }
   };

   namespace agent {
       using namespace marketsim::agent;

       template <Side SIDE>
        struct AgentT :
                OrderCanceller      < rng::Generator<Time>, boost::intrusive_ptr<order::LimitT<SIDE> >, 
                LinkToOrderBook     < OrderBook*, 
                SharedOrderPool     < boost::intrusive_ptr<order::LimitT<SIDE> >, 
                RefCounted          <
                HasWeakReferences   <
                AgentBase           < AgentT<SIDE> > 
                > > > >, always_0>
        {
            typedef 
                OrderCanceller      < rng::Generator<Time>, boost::intrusive_ptr<order::LimitT<SIDE> >, 
                LinkToOrderBook     < OrderBook*, 
                SharedOrderPool     < boost::intrusive_ptr< order::LimitT<SIDE> >, 
                RefCounted          <
                HasWeakReferences   <
                AgentBase           < AgentT<SIDE> > 
                > > > >, always_0>
                  base;
                              
            AgentT(OrderBook * book) 
                : base(boost::make_tuple(boost::make_tuple(dummy, book), new rng::constant<Time, rng::IGenerator<Time> >(1.))) 
            {}

            boost::intrusive_ptr<order::LimitT<SIDE> > sendOrder(Price p, Volume v)
            {
                boost::intrusive_ptr<order::LimitT<SIDE> > order = base::createOrder(pv(p,v));

                base::processOrder(order);

                return order;
            }

            void on_released() {}
        };
   }

    TEST_CASE("order_canceller", "")
    {
        Scheduler               scheduler;
        OrderBook               book;
        agent::AgentT<Sell>     ag(&book);

        {
            // 1. Testing empty order book
            scheduler.workTill(3.5);

            // 2. Testing cancellations
            LimitSellPtr L1 = ag.sendOrder(105, 5);
            LimitSellPtr L2 = ag.sendOrder(100, 5);
            LimitSellPtr L3 = ag.sendOrder(90, 5);

            assert(!book.empty<Sell>());
            assert(book.bestPrice<Sell>() == 90);

            scheduler.workTill(4.5);

            assert(!book.empty<Sell>());
            assert(!book.empty<Sell>());
            assert(book.bestPrice<Sell>() == 90);

            assert(L1->cancelled());
            assert(L1->cancelled());
            assert(!L2->cancelled());
            assert(!L3->cancelled());

            scheduler.workTill(5.5);

            assert(!book.empty<Sell>());
            assert(book.bestPrice<Sell>() == 100);
            assert(!L2->cancelled());
            assert(L3->cancelled());

            scheduler.workTill(6.5);

            assert(book.empty<Sell>());
            assert(L2->cancelled());

            // 3. Test a situation of order filling
            L1 = ag.sendOrder(90, 5);
            L2 = ag.sendOrder(105, 5);
            L3 = ag.sendOrder(100, 5);

            book.processOrder(order::MarketBuy(5));
            assert(!book.empty<Sell>());
            assert(book.bestPrice<Sell>() == 100);
            assert(L1->filled());

            // 4. Test of an external order cancellation
            L3->onCancelled();
            assert(!book.empty<Sell>());
            assert(book.bestPrice<Sell>() == 105);

            scheduler.workTill(7.5);
            assert(book.empty<Sell>());
            assert(L2->filled());
        }
    }
}}
