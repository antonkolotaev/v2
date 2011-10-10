#ifndef _marketsim_order_match_order_h_included_
#define _marketsim_order_match_order_h_included_

#include <marketsim/common_types.h>
namespace marketsim
{
    
    /// Matches incoming_order with a queue of limit orders stored in priority queue limit_orders
    /// \returns true iff incoming_order is completely matched
    template <
        typename OrderPriorityQueue,    // Limit orders priority queue with interface like std::priority_queue: pop, top, empty
        typename IncomingOrderPtr,      // a pointer to the incoming order which is to be matched
        typename OrderBook              // optional type of an order book which may be notified about order matching
    >
	    bool matchOrder(OrderPriorityQueue & limit_orders, IncomingOrderPtr & incoming_order, OrderBook * order_book)
    {
        /// while there are limit orders to match
	    while (!limit_orders.empty())
	    {
		    PriceVolume	matching;

            /// take the best one
            typename OrderPriorityQueue :: reference   limit_order = limit_orders.top();

            /// if it can be matched with the incoming order
            /// remember (Price,Volume) of the trade
		    if (incoming_order->canBeMatched(*limit_order, matching))
		    {
                /// notify the incoming order about matching
			    incoming_order->onMatched(matching, limit_order);
                /// notify the best limit order about matching
			    limit_order->onMatched(matching, incoming_order);

                /// if there is an order book, notify it too
                if (order_book)
                    order_book->onMatched(matching, *incoming_order, *limit_order);

                /// if the best limit order is completely filled, remove it
			    if (limit_order->filled())
				    limit_orders.pop();

                /// if the incoming order is completely filled, there's nothing to do anymore
			    if (incoming_order->filled())
				    return true;
		    }
		    else  // we cannot match the incoming order with the best limit order... so fail
			    return false;
	    }
	    // the limit order queue is empty... so fail
	    return false;
    }

    /// Dummy order book for matchOrder function 
    /// It should be used if 
    struct DummyOrderBook {
        template <typename IncomingOrder, typename LimitOrder>
            void onMatched(PriceVolume const &, IncomingOrder &, LimitOrder &)
            {}
    };

    /// 
    template <typename OrderPriorityQueue, typename IncomingOrder>
	    bool matchOrder(OrderPriorityQueue & limit_orders, IncomingOrder & incoming_order)
        {
            DummyOrderBook * dummy = 0;
            return matchOrder(limit_orders, incoming_order, dummy);
        }
}

#endif