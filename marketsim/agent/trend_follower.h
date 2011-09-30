#ifndef _marketsim_agent_trend_follower_h_included_
#define _marketsim_agent_trend_follower_h_included_

#include <boost/optional.hpp>
#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim
{
    template <typename IntervalDistr, typename VolumeDistr, typename Base>
        struct TrendFollower : Base 
    {
        typedef TrendFollower   base; // for derived typenamees

        template <typename T>
            TrendFollower(T const & x)
                :   Base(boost::get<0>(x))
                ,   timer_(*this, &TrendFollower::createOrder, boost::get<1>(x))
                ,   volume_(boost::get<2>(x))
            {}

    private:
        void createOrder()
        {
            Price midPrice2 = 
                Base::getOrderBook()->bestPrice<Sell>() + 
                base::getOrderBook()->bestPrice<Buy>();

            if (lastPrice_)
            {
                Price trend = midPrice2 - *lastPrice_;

                if (trend > 0)
                    Base::sendBuyOrder(Volume(volume_()));

                if (trend < 0)
                    Base::sendSellOrder(Volume(volume_()));
            }

            lastPrice_ = midPrice2;
        }

    private:
        typedef Timer<TrendFollower, IntervalDistr> timer_t;

        timer_t                 timer_;
        VolumeDistr             volume_;
        boost::optional<Price>  lastPrice_;
    };

}

#endif