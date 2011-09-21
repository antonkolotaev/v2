#include "stdafx.h"
#include "catch.hpp"

#include <queue>
#include <marketsim/match_order.h>
#include "orders.h"

namespace marketsim {
namespace {

    TEST_CASE("matching_engine", "Matching logic in an order queue")
    {
        std::priority_queue<
            LimitOrderSell, 
            std::vector<LimitOrderSell>, 
            LimitOrderSell::OrderedByPrice
        >	
        limit_orders;

        limit_orders.push(LimitOrderSell(pv(110, 3)));
        limit_orders.push(LimitOrderSell(pv(102, 3)));
        limit_orders.push(LimitOrderSell(pv(105, 3)));
        limit_orders.push(LimitOrderSell(pv(107, 3)));
        limit_orders.push(LimitOrderSell(pv(100, 3)));

        MarketOrderBuy m1 = 5;

        REQUIRE(matchOrder(limit_orders, m1));
        REQUIRE(m1.getExecutionHistory()[0] == pv(100u, 3u));
        REQUIRE(m1.getExecutionHistory()[1] == pv(102u, 2u));
        REQUIRE(limit_orders.top().price == 102);
        REQUIRE(limit_orders.top().volume == 1);

        LimitOrderBuy L1(pv(106, 10)); 
        REQUIRE(!matchOrder(limit_orders, L1));

        REQUIRE(L1.getExecutionHistory()[0] == pv(102u,1u));
        REQUIRE(L1.getExecutionHistory()[1] == pv(105u,3u));

        REQUIRE(L1.volume == 6);
        REQUIRE(limit_orders.top().price == 107);

        MarketOrderBuy m2 = (3);
        REQUIRE(matchOrder(limit_orders, m2));
        REQUIRE(m2.getExecutionHistory()[0] == pv(107u, 3u));
        REQUIRE(limit_orders.top().price == 110);

        MarketOrderBuy m3 = (5);
        REQUIRE(!matchOrder(limit_orders, m3));
        REQUIRE(m3.getExecutionHistory()[0] == pv(110u, 3u));
        REQUIRE(limit_orders.empty());
    }
}}