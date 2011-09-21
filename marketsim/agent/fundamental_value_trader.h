#ifndef _marketsim_agent_fundamental_value_trader_h_included_
#define _marketsim_agent_fundamental_value_trader_h_included_

#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim 
{
    template <class IntervalDistr, class VolumeDistr, class Base>
        struct FundamentalValueTrader : Base 
    {
        typedef FundamentalValueTrader   base; // for derived classes

        template <class T>
        FundamentalValueTrader(T const & x)
            :   Base    (boost::get<0>(x))
            ,   timer_  (*this, &FundamentalValueTrader::createOrder, boost::get<1>(x))
            ,   volume_           (boost::get<2>(x))
            ,   fundamentalValue2_(boost::get<3>(x) * 2)
        {}

    private:
        void createOrder()
        {
            Price midPrice2 = 
                Base::getOrderBook()->bestPrice<Sell>() + 
                base::getOrderBook()->bestPrice<Buy>();

            Price delta = midPrice2 - fundamentalValue2_;

            if (delta < 0)
                Base::processOrder(Base::createOrder(volume_(), buy_tag()));

            if (delta > 0)
                Base::processOrder(Base::createOrder(volume_(), sell_tag()));
        }

    private:
        typedef Timer<FundamentalValueTrader, IntervalDistr> timer_t;

        timer_t       timer_;
        VolumeDistr   volume_;
        Price const   fundamentalValue2_;
    };
}

#endif