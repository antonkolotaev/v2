
#include "catch.hpp"

#include <marketsim/order_queue/order_queue.h>
#include "orders.h"

namespace marketsim {
namespace {

    TEST_CASE("limit_buy_orders", "Ordering of buy limit orders")
    {
        using order::LimitOrderBuy;

        order_queue::OrderQueue<boost::intrusive_ptr<LimitOrderBuy> >	limit_orders;

        limit_orders.push(new LimitOrderBuy(pv(110, 3)));
        limit_orders.push(new LimitOrderBuy(pv(102, 3)));
        limit_orders.push(new LimitOrderBuy(pv(105, 3)));
        limit_orders.push(new LimitOrderBuy(pv(107, 3)));
        limit_orders.push(new LimitOrderBuy(pv(100, 3)));

		assert(limit_orders.volumeForBetterPrices(90) == 15);
		assert(limit_orders.volumeForBetterPrices(100) == 15);
		assert(limit_orders.volumeForBetterPrices(101) == 12);
		assert(limit_orders.volumeForBetterPrices(102) == 12);
		assert(limit_orders.volumeForBetterPrices(103) == 9);
		assert(limit_orders.volumeForBetterPrices(107) == 6);
		assert(limit_orders.volumeForBetterPrices(109) == 3);
		assert(limit_orders.volumeForBetterPrices(110) == 3);
		assert(limit_orders.volumeForBetterPrices(121) == 0);

		std::vector<PriceVolume>  best = limit_orders.getBestN(7);
		assert(best[0] == PriceVolume(110, 3));
		assert(best[1] == PriceVolume(107, 3));
		assert(best[2] == PriceVolume(105, 3));
		assert(best[3] == PriceVolume(102, 3));
		assert(best[4] == PriceVolume(100, 3));
		assert(best.size() == 5);

		assert(limit_orders.top()->getPrice() == 110);
    }


    TEST_CASE("limit_sell_orders", "Ordering of sell limit orders")
    {
        using order::LimitOrderSell;

        order_queue::OrderQueue<boost::intrusive_ptr<LimitOrderSell> >	limit_orders;

        limit_orders.push(new LimitOrderSell(pv(110, 3)));
		limit_orders.push(new LimitOrderSell(pv(100, 3)));
        limit_orders.push(new LimitOrderSell(pv(102, 3)));
        limit_orders.push(new LimitOrderSell(pv(105, 3)));
		limit_orders.push(new LimitOrderSell(pv(100, 3)));
        limit_orders.push(new LimitOrderSell(pv(107, 3)));
		limit_orders.push(new LimitOrderSell(pv(100, 3)));

		assert(limit_orders.volumeForBetterPrices(100) == 9);
		assert(limit_orders.volumeForBetterPrices(101) == 9);
		assert(limit_orders.volumeForBetterPrices(102) == 12);
		assert(limit_orders.volumeForBetterPrices(103) == 12);
		assert(limit_orders.volumeForBetterPrices(107) == 18);
		assert(limit_orders.volumeForBetterPrices(109) == 18);
		assert(limit_orders.volumeForBetterPrices(110) == 21);
		assert(limit_orders.volumeForBetterPrices(121) == 21);

		std::vector<PriceVolume>  best = limit_orders.getBestN(7);
		assert(best[0] == PriceVolume(100, 9));
		assert(best[1] == PriceVolume(102, 3));
		assert(best[2] == PriceVolume(105, 3));
		assert(best[3] == PriceVolume(107, 3));
		assert(best[4] == PriceVolume(110, 3));
		assert(best.size() == 5);

        assert(limit_orders.top()->getPrice() == 100);
    }
}}

