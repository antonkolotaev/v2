#ifndef _marketsim_order_market_order_base_h_included_
#define _marketsim_order_market_order_base_h_included_

#include <boost/mpl/if.hpp>

#include <marketsim/order/volume_holder.h>
#include <marketsim/order/side.h>
#include <marketsim/common/macros.h>

namespace marketsim {
namespace order     {
    /// Base class for limit orders
    /// \param SIDE side of the order
    /// \param Derived most derived class of the order; boost::mpl::na if there's no derived class
    template <Side SIDE, typename Derived = boost::mpl::na>    
        struct MarketOrderBase : VolumeHolder, side_is<SIDE>
    {
        explicit MarketOrderBase(Volume v) 
            :   VolumeHolder(v) 
        {}

        /// the most derived class of it
        typedef typename boost::mpl::if_na<Derived, MarketOrderBase>::type derived_t;

        /// order category tag: market
        typedef market_order_tag category;

        DECLARE_BASE(MarketOrderBase);
        DECLARE_ARROW(derived_t);

        /// Tries to match this order with a limit order
        /// Matching can be always done with a market order
        /// \return always true 
        /// (Price,Volume) of the possible trade is put to matching
        template <typename OtherOrderDerived>
            bool canBeMatched(
                LimitOrderBase<side_is<SIDE>::OTHER_SIDE, OtherOrderDerived> const & best_limit, 
                PriceVolume                                         & matching) const 
        {
            matching.volume = std::min(this->getVolume(), best_limit.getVolume());
            matching.price = best_limit.getPrice();
            return true;
        }
    };
}}

#endif
