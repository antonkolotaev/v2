#ifndef _marketsim_match_order_h_included_
#define _marketsim_match_order_h_included_

namespace marketsim
{
    /*
	    Matches incoming_order with a queue of limit orders stored in priority queue limit_orders
	    Returns true iff incoming_order has been matched
    */
    template <class OrderPriorityQueue, class IncomingOrder>
	    bool matchOrder(OrderPriorityQueue & limit_orders, IncomingOrder & incoming_order)
    {
	    while (!limit_orders.empty())
	    {
		    PriceVolume	matching;

            typename OrderPriorityQueue :: reference   limit_order = limit_orders.top();

		    if (incoming_order->canBeMatched(*limit_order, matching))
		    {
			    incoming_order->onMatched(matching, limit_order);
			    limit_order->onMatched(matching, incoming_order);

			    if (limit_order->filled())
				    limit_orders.pop();

			    if (incoming_order->filled())
				    return true;
		    }
		    else  // we cannot match the incoming order with the best limit order... so fail
			    return false;
	    }
	    // the limit order queue is empty... so fail
	    return false;
    }
}

#endif