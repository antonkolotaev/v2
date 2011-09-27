
#include "catch.hpp"

#include "orders.h"

namespace marketsim {
namespace {

    TEST_CASE("matching", "order matching logic")
    {
        PriceVolume res;
        REQUIRE(LimitOrderBuy(pv(110, 5))->canBeMatched(LimitOrderSell(pv(100, 3)), res));
        REQUIRE(res.price == 100);	REQUIRE(res.volume == 3);

        REQUIRE(LimitOrderSell(pv(100, 3))->canBeMatched(LimitOrderBuy(pv(110, 9)), res));
        REQUIRE(res.price == 110);	REQUIRE(res.volume == 3);

        REQUIRE(!LimitOrderSell(pv(110, 1))->canBeMatched(LimitOrderBuy(pv(100, 1)), res));
        REQUIRE(!LimitOrderBuy(pv(100, 1))->canBeMatched(LimitOrderSell(pv(110, 1)), res));

        REQUIRE(MarketOrderBuy(9)->canBeMatched(LimitOrderSell(pv(100, 3)), res));
        REQUIRE(res.price == 100);	REQUIRE(res.volume == 3); 

        REQUIRE(MarketOrderSell(2)->canBeMatched(LimitOrderBuy(pv(110, 8)), res));
        REQUIRE(res.price == 110);	REQUIRE(res.volume == 2);
    }
}}