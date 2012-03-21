#ifndef _marketsim_agent_noise_trader_h_included_
#define _marketsim_agent_noise_trader_h_included_

#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim {
namespace agent 
{
    /// Base class for agents encapsulating logic for noise trading
    /// In periods of time defined by IntervalDistr
    /// it generates a volume to trade using VolumeDistr 
    /// if this volume is positive it sells, if negative it buys
    template <
        typename IntervalDistr,     // generator for intervals between order generation
        typename VolumeDistr,       // generator for order volumes
        typename Base
    >
        struct NoiseTrader : Base 
    {
        DECLARE_BASE(NoiseTrader);

        /// 0-th argument is passed to the base class
        /// 1-th argument defines interval distribution between order generation
        /// 2-th argument defines order size distribution
        template <typename T>
            NoiseTrader(T const & x) 
                :   Base    (boost::get<0>(x))
                ,   timer_  (*this->self(), &NoiseTrader::createOrder, boost::get<1>(x))
                ,   volume_ (boost::get<2>(x))
            {}

    private:
        void createOrder()
        {
            Volume v = (Volume)volume_();

            if (v > 0)
                this->self()->template sendMarketOrder<Sell>(v);

            if (v < 0)
                this->self()->template sendMarketOrder<Buy>(-v);
        }

    private:
        typedef Timer<NoiseTrader, IntervalDistr>   timer_t;

        timer_t     timer_;
        VolumeDistr volume_;
    };

}}

#endif
