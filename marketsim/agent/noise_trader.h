#ifndef _marketsim_agent_noise_trader_h_included_
#define _marketsim_agent_noise_trader_h_included_

#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim
{
    // VolumeDistr -- volume distribution. if >0 the trader sells, if <0 the trader buys
    template <typename IntervalDistr, typename VolumeDistr, typename Base>
        struct NoiseTrader : Base 
    {
        typedef NoiseTrader     base; // for derived classes

        template <typename T>
            NoiseTrader(T const & x) 
                :   Base(boost::get<0>(x))
                ,   timer_(*self(), &NoiseTrader::createOrder, boost::get<1>(x))
                ,   volume_(boost::get<2>(x))
            {}

    private:
        void createOrder()
        {
            Volume v = volume_();

            if (v > 0)
                Base::processOrder(Base::createOrder(v, sell_tag()));

            if (v < 0)
                Base::processOrder(Base::createOrder(-v, buy_tag()));
        }

    private:
        typedef Timer<NoiseTrader, IntervalDistr>   timer_t;

        timer_t     timer_;
        VolumeDistr volume_;
    };

}

#endif