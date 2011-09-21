#include "stdafx.h"
#include "catch.hpp"

#include <marketsim/order/in_pool.h>
#include <marketsim/order_book.h>
#include <marketsim/object_pool.h>

#include "orders.h"

namespace marketsim {
namespace {

    template <Side SIDE>
       struct LimitT : 
                InPool<PlacedInPool, 
                    ExecutionHistory<
                        LimitOrderBase<SIDE, 
                            LimitT<SIDE> > > > 
    {
        LimitT(Price p, Volume v, object_pool<LimitT> * h) 
            :   base(boost::make_tuple(pv(p,v), h))
        {}
    };

    typedef LimitT<Buy>     LimitBuy;
    typedef LimitT<Sell>    LimitSell;

    struct LimitSellPool : object_pool<LimitSell> {}; 
    struct LimitBuyPool  : object_pool<LimitBuy>  {};

    typedef boost::intrusive_ptr<LimitBuy>  LimitOrderBuyPtr;
    typedef boost::intrusive_ptr<LimitSell> LimitOrderSellPtr;

    TEST_CASE("order_book", "Order book matching")
    {
        LimitSellPool  sell_orders;
        LimitBuyPool   buy_orders;

        OrderBook<LimitOrderBuyPtr, LimitOrderSellPtr>  book;

        for (int i = 0; i != 5; ++i)
        {
            book.processOrder(new (sell_orders.alloc()) LimitSell (104 - i, 5 - i, &sell_orders));
            book.processOrder(new (buy_orders.alloc()) LimitBuy   (95 + i, 6 - i, &buy_orders));
        }

        REQUIRE(!book.empty<Sell>());
        REQUIRE(!book.empty<Buy>());

        REQUIRE(book.bestPrice<Sell>() == 100);
        REQUIRE(book.bestVolume<Sell>() == 1);

        REQUIRE(book.bestPrice<Buy>() == 99);
        REQUIRE(book.bestVolume<Buy>() == 2);

        ExecutionHistoryStorage::const_iterator it;

        LimitBuy * lb;
        book.processOrder(lb = new (buy_orders.alloc()) LimitBuy   (100, 2, &buy_orders));

        REQUIRE(book.bestPrice<Sell>() == 101);
        REQUIRE(book.bestVolume<Sell>() == 2);

        REQUIRE(book.bestPrice<Buy>() == 100);
        REQUIRE(book.bestVolume<Buy>() == 1);

        it = lb->getExecutionHistory().begin();
        REQUIRE(lb->getExecutionHistory()[0] == pv(100u,1u));

        LimitSell * ls;
        book.processOrder(ls = new (sell_orders.alloc()) LimitSell   (99, 5, &sell_orders));
        REQUIRE(book.bestPrice<Sell>() == 99);
        REQUIRE(book.bestVolume<Sell>() == 2);

        REQUIRE(book.bestPrice<Buy>() == 98);
        REQUIRE(book.bestVolume<Buy>() == 3);

        REQUIRE(ls->getExecutionHistory()[0] == pv(100u,1u));
        REQUIRE(ls->getExecutionHistory()[1] == pv(99u,2u));

        MarketOrderSell ms(5);
        book.processOrder(&ms);

        REQUIRE(book.bestPrice<Buy>() == 97);
        REQUIRE(book.bestVolume<Buy>() == 2);

        REQUIRE(ms.getExecutionHistory()[0] == pv(98u,3u));
        REQUIRE(ms.getExecutionHistory()[1] == pv(97u,2u));

        MarketOrderBuy mb(3);
        book.processOrder(&mb);

        REQUIRE(book.bestPrice<Sell>() == 101);
        REQUIRE(book.bestVolume<Sell>() == 1);
        REQUIRE(mb.getExecutionHistory()[0] == pv(99u,2u));
        REQUIRE(mb.getExecutionHistory()[1] == pv(101u,1u));

        //std::cout << "-----\n";
    }
}}