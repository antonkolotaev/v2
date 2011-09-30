#ifndef _marketsim_agent_signal_trader_h_included_
#define _marketsim_agent_signal_trader_h_included_

namespace marketsim
{
    template <typename VolumeDistr, typename Base>
        struct SignalTrader : Base
    {
        DECLARE_BASE(SignalTrader);

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
                self()->sendSellOrder((Volume)volume_());
            }

            if (x < -threshold_)
            {
                self()->sendBuyOrder((Volume)volume_());
            }
        }

    private:
        VolumeDistr     volume_;
        double          threshold_;
    };
}

#endif