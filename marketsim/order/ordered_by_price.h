#ifndef _marketsim_order_ordered_by_price_h_included_
#define _marketsim_order_ordered_by_price_h_included_

#include <boost/intrusive_ptr.hpp>

namespace marketsim {
namespace order     {
    /// -------------------- ordered_by_price 

    /// meta function returning a binary predicate for comparing Orders by price
    /// (sell order with smaller price is better; buy order with greater price is better)
    template <typename Order>
        struct ordered_by_price 
    {
        /// by default we rely on OrderedByPrice inner type defined within Order
        typedef typename Order :: OrderedByPrice type;
    };

    /// ordered_by_price for boost::intrusive_ptr<Order> dereferences passed pointers 
    /// and applies ordered_by_price<Order>::type
    template <typename Order> struct ordered_by_price<boost::intrusive_ptr<Order> > 
    {
        typedef typename boost::intrusive_ptr<Order>    OrderPtr;
        typedef typename ordered_by_price<Order>::type  OrderCmp;

        struct type 
        {
			bool operator () (OrderPtr const & lhs, OrderPtr const & rhs) const 
			{
				return OrderCmp()(*lhs, *rhs);
			}
			bool operator () (OrderPtr const & lhs, Price rhs) const 
			{
				return OrderCmp()(*lhs, rhs);
			}
        };
    };
}}

#endif