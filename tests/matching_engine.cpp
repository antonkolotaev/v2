
#include "catch.hpp"

#include <queue>
#include <marketsim/order/match_order.h>
#include <marketsim/order/ordered_by_price.h>
#include "orders.h"

namespace marketsim {
namespace {

    TEST_CASE("matching_engine", "Matching logic in an order queue")
    {
        using order::LimitOrderSell;
        using order::LimitOrderBuy;
        using order::MarketOrderBuy;

        Scheduler scheduler;

        std::priority_queue<
            boost::intrusive_ptr<LimitOrderSell>, 
            std::vector<boost::intrusive_ptr<LimitOrderSell> >, 
            order::ordered_by_price<boost::intrusive_ptr<LimitOrderSell> >::type
        >	
        limit_orders;

        limit_orders.push(new LimitOrderSell(pv(110, 3)));
        limit_orders.push(new LimitOrderSell(pv(102, 3)));
        limit_orders.push(new LimitOrderSell(pv(105, 3)));
        limit_orders.push(new LimitOrderSell(pv(107, 3)));
        limit_orders.push(new LimitOrderSell(pv(100, 3)));

        MarketOrderBuy m1 = 5;

        assert(matchOrder(limit_orders, m1));
        assert(m1.getExecutionHistory()[0].value == pv(100u, 3u));
        assert(m1.getExecutionHistory()[1].value == pv(102u, 2u));
        assert(limit_orders.top()->getPrice() == 102);
        assert(limit_orders.top()->getVolume() == 1);

        LimitOrderBuy L1(pv(106, 10)); 
        assert(!matchOrder(limit_orders, L1));

        assert(L1.getExecutionHistory()[0].value == pv(102u,1u));
        assert(L1.getExecutionHistory()[1].value == pv(105u,3u));

        assert(L1.getVolume() == 6);
        assert(limit_orders.top()->getPrice() == 107);

        MarketOrderBuy m2 = (3);
        assert(matchOrder(limit_orders, m2));
        assert(m2.getExecutionHistory()[0].value == pv(107u, 3u));
        assert(limit_orders.top()->getPrice() == 110);

        MarketOrderBuy m3 = (5);
        assert(!matchOrder(limit_orders, m3));
        assert(m3.getExecutionHistory()[0].value == pv(110u, 3u));
        assert(limit_orders.empty());
    }
}}