#include "stdafx.h"
#include "catch.hpp"

#include <queue>
#include "orders.h"

namespace marketsim {
namespace {

    TEST_CASE("limit_buy_orders", "Ordering of buy limit orders")
    {
        std::priority_queue<LimitOrderBuy, std::vector<LimitOrderBuy>, LimitOrderBuy::OrderedByPrice>	limit_orders;

        limit_orders.push(LimitOrderBuy(pv(110, 3)));
        limit_orders.push(LimitOrderBuy(pv(102, 3)));
        limit_orders.push(LimitOrderBuy(pv(105, 3)));
        limit_orders.push(LimitOrderBuy(pv(107, 3)));
        limit_orders.push(LimitOrderBuy(pv(100, 3)));

        REQUIRE(limit_orders.top().price == 110);
    }

    TEST_CASE("limit_sell_orders", "Ordering of sell limit orders")
    {
        std::priority_queue<LimitOrderSell, std::vector<LimitOrderSell>, LimitOrderSell::OrderedByPrice>	limit_orders;

        limit_orders.push(LimitOrderSell(pv(110, 3)));
        limit_orders.push(LimitOrderSell(pv(102, 3)));
        limit_orders.push(LimitOrderSell(pv(105, 3)));
        limit_orders.push(LimitOrderSell(pv(107, 3)));
        limit_orders.push(LimitOrderSell(pv(100, 3)));

        REQUIRE(limit_orders.top().price == 100);
    }
}}

