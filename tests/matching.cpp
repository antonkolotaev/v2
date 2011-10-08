
#include "catch.hpp"

#include "orders.h"

namespace marketsim {
namespace {

    TEST_CASE("matching", "order matching logic")
    {
        using namespace order;

        PriceVolume res;
        assert(LimitOrderBuy(pv(110, 5))->canBeMatched(LimitOrderSell(pv(100, 3)), res));
        assert(res.price == 100);	assert(res.volume == 3);

        assert(LimitOrderSell(pv(100, 3))->canBeMatched(LimitOrderBuy(pv(110, 9)), res));
        assert(res.price == 110);	assert(res.volume == 3);

        assert(!LimitOrderSell(pv(110, 1))->canBeMatched(LimitOrderBuy(pv(100, 1)), res));
        assert(!LimitOrderBuy(pv(100, 1))->canBeMatched(LimitOrderSell(pv(110, 1)), res));

        assert(MarketOrderBuy(9)->canBeMatched(LimitOrderSell(pv(100, 3)), res));
        assert(res.price == 100);	assert(res.volume == 3); 

        assert(MarketOrderSell(2)->canBeMatched(LimitOrderBuy(pv(110, 8)), res));
        assert(res.price == 110);	assert(res.volume == 2);
    }
}}