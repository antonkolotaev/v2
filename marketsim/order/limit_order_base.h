#ifndef _marketsim_order_limit_order_base_h_included_
#define _marketsim_order_limit_order_base_h_included_

#include <boost/mpl/if.hpp>

#include <marketsim/common/macros.h>
#include <marketsim/order/volume_holder.h>
#include <marketsim/order/price_holder.h>

namespace marketsim {
namespace order     {
    /// Base class for limit orders
    /// \param SIDE side of the order
    /// \param Derived most derived class of the order; boost::mpl::na if there's no derived class
    template <Side SIDE, typename Base>
        struct LimitOrderBase : VolumeHolder, PriceHolder<SIDE>, Base
    {
        typedef PriceHolder<SIDE> PH;
        typedef VolumeHolder      VH;
        
        /// constructs a limit order for (Price,Volume) pair
        explicit LimitOrderBase(PriceVolume const & pv) 
            :   PH(pv.price)
            ,   VH(pv.volume) 
        {}

        /// \return (Price,Volume) pair for the order
        PriceVolume getPV() const { return PriceVolume(PH::getPrice(), VH::getVolume()); }

        /// category of the order: limit
        typedef limit_order_tag category;

        /// the most derived class
        //typedef typename boost::mpl::if_na<Derived, LimitOrderBase>::type derived_t;

        //DECLARE_ARROW(derived_t);

        /// Tries to match this order with another limit order
        /// Matching can be done iff they have crossing prices
        /// \return true iff the orders can be matched
        /// in this case (Price,Volume) of the possible trade is put to matching
        template <typename OtherOrderDerived>
            bool canBeMatched(
                LimitOrderBase<PH::OTHER_SIDE, OtherOrderDerived> const & best_limit, 
                PriceVolume                                         & matching) const 
        {
            if (!compare_prices(this->getPrice(), best_limit.getPrice()))
            {
                matching.price  = best_limit.getPrice();
                matching.volume = std::min(this->getVolume(), best_limit.getVolume());
                return true;
            }
            return false;
        }

        friend std::ostream & operator << (std::ostream & out, LimitOrderBase const & p)
        {
            return out << "(" << p.getVolume() << " @ " << p.getPrice() << ")";
        }
    };

}}

#endif
