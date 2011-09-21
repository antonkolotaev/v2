#ifndef _marketsim_order_book_h_included_
#define _marketsim_order_book_h_included_

#include <marketsim/order_queue.h>
#include <marketsim/match_order.h>

namespace marketsim
{
    template <
        class LimitOrderBuy, 
        class LimitOrderSell
    >
        struct OrderBook
    {
        OrderBook(Price tickSize = 1) : tick_size_(tickSize) {}

        Price getTickSize() const { return tick_size_; }

        template <class Order>
            bool processOrder(Order order, limit_order_tag)
            {
                typedef typename order_side<Order> :: type      side;
                typedef typename opposite_side<side> :: type    opposite;

                if (!matchOrder(orderQueue(opposite()), order))
                {
                    orderQueue(side()).push(order);
                    return false;
                }

                return true;
            }

        template <class Order>
            bool processOrder(Order order, market_order_tag)
            {
                typedef typename order_side<Order> :: type      side;
                typedef typename opposite_side<side> :: type    opposite;

                return matchOrder(orderQueue(opposite()), order);
            }

        template <class Order>
            bool processOrder(Order order)
            {
                return processOrder(order, typename order_category<Order>::type());
            }


        template <Side SIDE> bool   empty(side_tag<SIDE> x = side_tag<SIDE>()) /*const*/ { return orderQueue(side_tag<SIDE>()).empty(); }
        template <Side SIDE> Volume bestVolume(side_tag<SIDE> x = side_tag<SIDE>()) /*const*/ { return orderQueue(side_tag<SIDE>()).top()->volume; }
        template <Side SIDE> Price  bestPrice(side_tag<SIDE> x = side_tag<SIDE>()) /*const*/ { return orderQueue(side_tag<SIDE>()).top()->price; }

        OrderQueue<LimitOrderBuy>  & orderQueue(buy_tag) { return buy_side_; }
        OrderQueue<LimitOrderSell> & orderQueue(sell_tag){ return sell_side_; }

        OrderQueue<LimitOrderBuy> const& orderQueue(buy_tag) const { return buy_side_; }
        OrderQueue<LimitOrderSell>const& orderQueue(sell_tag)const { return sell_side_; }

    private:
        Price  const                 tick_size_;
        OrderQueue<LimitOrderBuy>    buy_side_;
        OrderQueue<LimitOrderSell>   sell_side_;
    };

}

#endif