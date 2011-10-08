#ifndef _marketsim_agent_liquidity_provider_h_included_
#define _marketsim_agent_liquidity_provider_h_included_

#include <marketsim/object_pool.h>
#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim {
namespace agent 
{
    /// Base class for agents encapsulating logic of a one-side liquidity provider
    /// In some periods of time (given by GenerationPeriod) 
    /// it chooses using LagDistribution a delta to the current best price in the market
    /// and generates a limit order with this price and volume chosen according to VolumeDistribution
    template <
        typename GenerationPeriod,      // distribution of intervals between order generation
        typename LagDistribution,       // distribution of price deltas
        typename VolumeDistribution,    // distribution of volume for orders
        typename Base
    >
        struct LiquidityProvider : Base
    {
        /// 0-th argument is passed to the base class
        /// 1-th argument is a generator of time intervals between order generations
        /// 2-th argument is a generator for price deltas with respect to the current best market price
        /// 3-th argument is a generator for order volumes
        /// 4-th argument is an initial price for asset which is used when no orders are in market
        template <typename T> 
            LiquidityProvider(T const & x) 
            :   Base         (boost::get<0>(x))
            ,   timer_       (*self(), &LiquidityProvider::sendOrder, boost::get<1>(x))
            ,   lag_         (boost::get<2>(x))
            ,   volume_      (boost::get<3>(x))
            ,   initialPrice_(boost::get<4>(x)) 
        {}

        typedef typename Base::order_type order_type;

        typedef typename order_side<order_type> :: type  side_tag;

        DECLARE_BASE(LiquidityProvider)

        void sendOrder()
        {
            Price bp = getOrderBook()->empty(side_tag()) ? initialPrice_ : getOrderBook()->bestPrice(side_tag());
            Price delta = getOrderBook()->ceilPrice(lag_());
            Price p = std::max(Price(1), side_tag::value == Sell ? bp + delta : bp - delta);
            
            Volume v = std::max(Volume(1), static_cast<Volume>(volume_()));

            Base::processOrder(Base::createOrder(pv(p,v)));
        }

    private:
        typedef Timer<LiquidityProvider, GenerationPeriod>  timer_t;

        timer_t             timer_;
        LagDistribution     lag_;
        VolumeDistribution  volume_;
        Price const         initialPrice_;
    };

}}

#endif