#ifndef _marketsim_common_types_h_included
#define _marketsim_common_types_h_included

#include <boost/intrusive_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <sstream>

namespace marketsim
{
    /// converts an object to std::string by calling its operator << 
    /// analogous to boost::lexical_cast<std::string>
    template <typename T>
        std::string toStr(T const &x) 
    {
        return (std::stringstream() << x).str();
    }
}

#include <marketsim/common/macros.h>
#include <marketsim/common/PriceVolume.h>
#include <marketsim/common/traits.h>

#pragma warning ( disable : 4355 )


namespace marketsim 
{
    /// -------------------- ordered_by_price 
    /// to be moved to order/ordered_by_price

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

    /// ------------------------------- miscellaneous
    /// an empty base class
	struct Empty {};

    /// a dummy value 
    /// we use it in order to pass something to a function requiring a parameter
    enum Dummy { dummy };


}

#endif