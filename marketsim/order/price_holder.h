#ifndef _marketsim_order_price_holder_h_included_
#define _marketsim_order_price_holder_h_included_

#include <marketsim/order/side.h>

namespace marketsim {
namespace order     {

    /// Base class for orders encapsulating logic related to the price of the order
	template <Side SIDE>	
        struct PriceHolder : side_is<SIDE>
	{
		PriceHolder(Price p) : price_(p) {}

        /// \returns true iff lhs is worse than rhs, i.e.
        /// lhs has smaller price than rhs in case of buy order and 
        /// rhs has greater price than rhs in case of sell order
        static bool compare_prices(Price lhs, Price rhs)
        {
            return SIDE == Buy ? lhs < rhs : lhs > rhs;
        }

        /// Binary predicate comparing two prices with respect of the side of the order
        struct OrderedByPrice 
        {
			bool operator () (PriceHolder const & lhs, PriceHolder const & rhs) const 
			{
				return compare_prices(lhs.price_, rhs.price_);
			}
			bool operator () (PriceHolder const & lhs, Price rhs) const
			{
				return compare_prices(lhs.price_, rhs);
			}
			bool operator () (Price lhs, PriceHolder const & rhs) const 
			{
				return compare_prices(lhs, rhs.price_);
			}
			bool operator () (Price lhs, Price rhs) const 
			{
				return compare_prices(lhs, rhs);
			}
        };

        Price getPrice() const { return price_; }
    private:
		Price const	price_;  
	};

}}

#endif