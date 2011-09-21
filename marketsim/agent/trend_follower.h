#ifndef _marketsim_agent_trend_follower_h_included_
#define _marketsim_agent_trend_follower_h_included_

#include <boost/optional.hpp>
#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim
{
    template <class IntervalDistr, class VolumeDistr, class Base>
        struct TrendFollower : Base 
    {
        typedef TrendFollower   base; // for derived classes

        template <class T>
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
                    Base::processOrder(Base::createOrder(volume_(), buy_tag()));

                if (trend < 0)
                    Base::processOrder(Base::createOrder(volume_(), sell_tag()));
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