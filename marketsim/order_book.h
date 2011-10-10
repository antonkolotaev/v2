#ifndef _marketsim_order_book_h_included_
#define _marketsim_order_book_h_included_

#include <marketsim/order_queue.h>
#include <marketsim/order/match_order.h>

namespace marketsim
{
    template <
        typename QueueBuySide, 
        typename QueueSellSide
    >
        struct OrderBook
    {
        OrderBook(Price tickSize = 1) : tick_size_(tickSize) {}

        Price floorPrice(PriceF p) const 
        {
            return Price(floor(p / tick_size_)) * tick_size_;
        }

        Price ceilPrice(PriceF p) const 
        {
            return Price(ceil(p / tick_size_)) * tick_size_;
        }

		typedef OrderBook	base;

		typedef typename QueueBuySide :: value_type		LimitOrderBuy;
		typedef typename QueueSellSide:: value_type		LimitOrderSell;

        Price getTickSize() const { return tick_size_; }

		DECLARE_ARROW(OrderBook);	// to be replaced by derived

        template <typename Order>
            bool processOrder_impl(Order order, limit_order_tag)
            {
                typedef typename order_side<Order> :: type      side;
                typedef typename opposite_side<side> :: type    opposite;

                if (!matchOrder(orderQueue(opposite()), order, self()))
                {
                    orderQueue(side()).push(order);
                    return false;
                }

                return true;
            }

        template <typename Order>
            bool processOrder_impl(Order order, market_order_tag)
            {
                typedef typename order_side<Order> :: type      side;
                typedef typename opposite_side<side> :: type    opposite;

                return matchOrder(orderQueue(opposite()), order, self());
            }

        template <typename Order>
            bool processOrder(Order order)
            {
                return processOrder_impl(order, typename order_category<Order>::type());
            }

        template <typename Order>
            void onOrderCancelled(Order order, limit_order_tag)
            {
                typedef typename order_side<Order> :: type      side;
                orderQueue(side()).onOrderCancelled(order);
            }

        template <typename Order>
            void onOrderCancelled(Order order)
            {
                onOrderCancelled(order, typename order_category<Order>::type());
            }

        // we will use this method to record the last trade
        template <typename IncomingOrder, typename LimitOrder>
            void onMatched(PriceVolume const &x, IncomingOrder &incoming_order, LimitOrder &limit_order)
            {
				typedef typename order_side<LimitOrder> :: type      side;
				orderQueue(side()).onPartiallyFilled(limit_order, x);
			}


        template <Side SIDE> bool   empty(side_tag<SIDE> x = side_tag<SIDE>()) /*const*/ { return orderQueue(side_tag<SIDE>()).empty(); }

        // 
        template <Side SIDE> Volume bestVolume(side_tag<SIDE> x = side_tag<SIDE>()) /*const*/ { return orderQueue(side_tag<SIDE>()).getBestVolume(); }
        template <Side SIDE> Price  bestPrice(side_tag<SIDE> x = side_tag<SIDE>()) /*const*/ { return orderQueue(side_tag<SIDE>()).getBestPrice(); }

        QueueBuySide  & orderQueue(buy_tag) { return buy_side_; }
        QueueSellSide & orderQueue(sell_tag){ return sell_side_; }

        QueueBuySide  const& orderQueue(buy_tag) const { return buy_side_; }
        QueueSellSide const& orderQueue(sell_tag)const { return sell_side_; }

#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T>
            static void py_visit(T & class_def)
            {
                class_def
                    .def_readonly("asks", &OrderBook::sell_side_)
                    .def_readonly("bids", &OrderBook::buy_side_)
                    .def_readwrite("tickSize", &OrderBook::tick_size_)
                    .def("floorPrice", &OrderBook::floorPrice)
                    .def("ceilPrice", &OrderBook::ceilPrice)
                    ;
            }
#endif
    private:
        OrderBook(OrderBook const&);
    private:
        Price           tick_size_;
        QueueBuySide    buy_side_;
        QueueSellSide   sell_side_;
    };

}

#endif