#ifndef _marketsim_order_book_h_included_
#define _marketsim_order_book_h_included_

#include <marketsim/order_queue/order_queue.h>
#include <marketsim/order/match_order.h>

namespace marketsim
{
    /// OrderBook as a pair of a sell-side order queue and a buy-side order queue
    /// Also it has a tick size (which can be factored out to a another class)
    template <
        typename QueueBuySide,  
        typename QueueSellSide
    >
        struct OrderBook
    {
        OrderBook(Price tickSize = 1) : tick_size_(tickSize) {}

        /// floors a floating point price to the closest tick
        Price floorPrice(PriceF p) const 
        {
            return Price(floor(p / tick_size_)) * tick_size_;
        }

        /// ceils a floating point price to the closest tick
        Price ceilPrice(PriceF p) const 
        {
            return Price(ceil(p / tick_size_)) * tick_size_;
        }

		DECLARE_BASE(OrderBook);

        /// \return the tick size 
        Price getTickSize() const { return tick_size_; }

		DECLARE_ARROW(OrderBook);	// to be replaced by derived

        /// processes an order (matches and stores it if it is a limit one)
        template <typename Order>
            bool processOrder(Order order)
            {
                return processOrder_impl(order, typename order_category<Order>::type());
            }

        /// notification that an order has been cancelled
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


        /// \return true if order queue of the given side is empty
        template <Side SIDE> bool   empty(side_tag<SIDE> x = side_tag<SIDE>()) /*const*/ 
        {
            return orderQueue(side_tag<SIDE>()).empty(); 
        }

        // \return volume of orders with the best price at the queue of the given price
        template <Side SIDE> Volume bestVolume(side_tag<SIDE> x = side_tag<SIDE>()) /*const*/ 
        {
            return orderQueue(side_tag<SIDE>()).getBestVolume(); 
        }
        
        // \return price of the best order at the queue of the given price
        template <Side SIDE> Price  bestPrice(side_tag<SIDE> x = side_tag<SIDE>()) /*const*/ 
        { 
            return orderQueue(side_tag<SIDE>()).getBestPrice(); 
        }

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

        /// processes limit orders
        template <typename Order>
            bool processOrder_impl(Order order, limit_order_tag)
            {
                /// a tag for the order's side
                typedef typename order_side<Order> :: type      side;
                typedef typename opposite_side<side> :: type    opposite;

                if (!matchOrder(orderQueue(opposite()), order, self()))
                {
                    orderQueue(side()).push(order);
                    return false;
                }

                return true;
            }

        /// processes market orders
        template <typename Order>
            bool processOrder_impl(Order order, market_order_tag)
            {
                typedef typename order_side<Order> :: type      side;
                typedef typename opposite_side<side> :: type    opposite;

                return matchOrder(orderQueue(opposite()), order, self());
            }

        /// cancels a limit order
        template <typename Order>
            void onOrderCancelled(Order order, limit_order_tag)
            {
                typedef typename order_side<Order> :: type      side;
                orderQueue(side()).onOrderCancelled(order);
            }


    private:
        Price           tick_size_;
        QueueBuySide    buy_side_;
        QueueSellSide   sell_side_;
    };

}

#endif