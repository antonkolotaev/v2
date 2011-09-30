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
        DECLARE_BASE(NoiseTrader);

        template <typename T>
            NoiseTrader(T const & x) 
                :   Base(boost::get<0>(x))
                ,   timer_(*self(), &NoiseTrader::createOrder, boost::get<1>(x))
                ,   volume_(boost::get<2>(x))
            {}

#ifdef MARKETSIM_BOOST_PYTHON

        template <typename T>
            static void py_visit(T &c)
            {
                Base::py_visit(c);
            }

#endif

    private:
        void createOrder()
        {
            Volume v = (Volume)volume_();

            if (v > 0)
                self()->sendSellOrder(v);

            if (v < 0)
                self()->sendBuyOrder(-v);
        }

    private:
        typedef Timer<NoiseTrader, IntervalDistr>   timer_t;

        timer_t     timer_;
        VolumeDistr volume_;
    };

}

#endif