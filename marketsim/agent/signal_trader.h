#ifndef _marketsim_agent_signal_trader_h_included_
#define _marketsim_agent_signal_trader_h_included_

namespace marketsim {
namespace agent 
{
    /// Base class for agents encapsulating signal trading logic
    /// if a value of a signal is greater than some threshold, the trader sells
    /// if a value of a signal is less than -threshold, the trader buys
    /// Amount of assets traded is defined by VolumeDistr
    template <
        typename VolumeDistr,       /// generator for order volumes
        typename Base
    >
        struct SignalTrader : Base
    {
        DECLARE_BASE(SignalTrader);

        /// 0-th argument is passed to the base class
        /// 1-th argument defines order size distribution
        /// 2-th argument defines The Threshold
        template <typename T>
            SignalTrader(T const & x)
                :   Base        (boost::get<0>(x))
                ,   volume_     (boost::get<1>(x))
                ,   threshold_  (boost::get<2>(x))
            {}

        void onSignal(double x)
        {
            if (x > threshold_)
            {
                self()->sendMarketOrder<Sell>((Volume)volume_());
            }

            if (x < -threshold_)
            {
                self()->sendMarketOrder<Buy>((Volume)volume_());
            }
        }

    private:
        VolumeDistr     volume_;
        double          threshold_;
    };
}}

#endif