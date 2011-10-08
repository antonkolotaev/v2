#ifndef _marketsim_agent_trend_follower_h_included_
#define _marketsim_agent_trend_follower_h_included_

#include <boost/optional.hpp>
#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim {
namespace agent 
{
    /// Base class for agents encapsulating logic for a trend follower trader
    /// In some moments of time (defined by IntervalDistr)
    /// it evaluates current trend in the market
    /// if the trend is positive, it buys
    /// if the trend is negative, it sells
    /// Amount of assets to be traded is defined by VolumeDistr
    template <
        typename IntervalDistr,     // generator for intervals between order creation
        typename VolumeDistr,       // generator for order volumes
        typename Base
    >
        struct TrendFollower : Base 
    {
        DECLARE_BASE(TrendFollower);

        /// 0-th argument is passed to the base class
        /// 1-th argument defines interval distribution between order generation
        /// 2-th argument defines order size distribution
        template <typename T>
            TrendFollower(T const & x)
                :   Base    (boost::get<0>(x))
                ,   timer_  (*this, &TrendFollower::createOrder, boost::get<1>(x))
                ,   volume_ (boost::get<2>(x))
            {}

    private:
        void createOrder()
        {
            if (!self()->getOrderBook()->empty<Sell>() && !self()->getOrderBook()->empty<Buy>())
            {
                Price midPrice2 = 
                    self()->getOrderBook()->bestPrice<Sell>() + 
                    self()->getOrderBook()->bestPrice<Buy>();

                if (lastPrice_)
                {
                    Price trend = midPrice2 - *lastPrice_;
                    Volume v = (Volume)volume_();

                    if (trend > 0)
                        Base::sendMarketOrder<Buy>(v);

                    if (trend < 0)
                        Base::sendMarketOrder<Sell>(v);
                }

                lastPrice_ = midPrice2;
            }
        }

    private:
        typedef Timer<TrendFollower, IntervalDistr> timer_t;

        timer_t                 timer_;
        VolumeDistr             volume_;
        boost::optional<Price>  lastPrice_;
    };

}}

#endif