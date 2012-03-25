
#include "catch.hpp"

#include <marketsim/order/in_pool.h>
#include <marketsim/order_book.h>
#include <marketsim/object_pool.h>

#include "orders.h"

namespace marketsim {
namespace {

    namespace order 
    {
        using namespace marketsim::order;

        template <Side SIDE>
           struct LimitT : 
                    InPool           < PlacedInPool, 
                    ExecutionHistory <
                    LimitOrderBase   < SIDE, 
                    RefCounted       <
                    derived_is       <
                    LimitT           < SIDE
                    > > > > > >
        {
            typedef 
                    InPool           < PlacedInPool, 
                    ExecutionHistory <
                    LimitOrderBase   < SIDE, 
                    RefCounted       <
                    derived_is       <
                    LimitT           < SIDE
                    > > > > > >
                  base;
                              
            LimitT(Price p, Volume v, object_pool<LimitT> * h) 
                :   base(boost::make_tuple(pv(p,v), h))
            {}
        };
    }

    typedef order::LimitT<Buy>     LimitBuy;
    typedef order::LimitT<Sell>    LimitSell;

    struct LimitSellPool : object_pool<LimitSell> {}; 
    struct LimitBuyPool  : object_pool<LimitBuy>  {};

    typedef boost::intrusive_ptr<LimitBuy>  LimitOrderBuyPtr;
    typedef boost::intrusive_ptr<LimitSell> LimitOrderSellPtr;

    TEST_CASE("order_book", "Order book matching")
    {
        Scheduler scheduler;

        LimitSellPool  sell_orders;
        LimitBuyPool   buy_orders;

        OrderBook<
            order_queue::OrderQueue<LimitOrderBuyPtr>, 
            order_queue::OrderQueue<LimitOrderSellPtr> 
        >  book;

        for (int i = 0; i != 5; ++i)
        {
            book.processOrder(new (sell_orders.alloc()) LimitSell (104 - i, 5 - i, &sell_orders));
            book.processOrder(new (buy_orders.alloc()) LimitBuy   (95 + i, 6 - i, &buy_orders));
        }

        assert(!book.empty<Sell>());
        assert(!book.empty<Buy>());

        assert(book.bestPrice<Sell>() == 100);
        assert(book.bestVolume<Sell>() == 1);

        assert(book.bestPrice<Buy>() == 99);
        assert(book.bestVolume<Buy>() == 2);

        order::ExecutionHistoryStorage::const_iterator it;

        LimitBuy * lb;
        book.processOrder(lb = new (buy_orders.alloc()) LimitBuy   (100, 2, &buy_orders));

        assert(book.bestPrice<Sell>() == 101);
        assert(book.bestVolume<Sell>() == 2);

        assert(book.bestPrice<Buy>() == 100);
        assert(book.bestVolume<Buy>() == 1);

        it = lb->getExecutionHistory().begin();
        assert(it[0].value == pv(100u,1u));

        LimitSell * ls;
        book.processOrder(ls = new (sell_orders.alloc()) LimitSell   (99, 5, &sell_orders));
        assert(book.bestPrice<Sell>() == 99);
        assert(book.bestVolume<Sell>() == 2);

        assert(book.bestPrice<Buy>() == 98);
        assert(book.bestVolume<Buy>() == 3);

        it = ls->getExecutionHistory().begin();
        assert(it[0].value == pv(100u,1u));
        assert(it[1].value == pv(99u,2u));

        order::MarketOrderSell ms(5);
        book.processOrder(&ms);

        assert(book.bestPrice<Buy>() == 97);
        assert(book.bestVolume<Buy>() == 2);

        it = ms.getExecutionHistory().begin();
        assert(it[0].value == pv(98u,3u));
        assert(it[1].value == pv(97u,2u));

        order::MarketOrderBuy mb(3);
        book.processOrder(&mb);

        assert(book.bestPrice<Sell>() == 101);
        assert(book.bestVolume<Sell>() == 1);
        it = mb->getExecutionHistory().begin();
        assert(it[0].value == pv(99u,2u));
        assert(it[1].value == pv(101u,1u));

        //std::cout << "-----\n";
    }
}}
