#ifndef _marketsim_agent_signal_trader_h_included_
#define _marketsim_agent_signal_trader_h_included_

namespace marketsim
{
    template <typename VolumeDistr, typename Base>
        struct SignalTrader : Base
    {
        typedef SignalTrader    base; // for derived typenamees

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
                Base::processOrder(Base::createOrder(volume_(), sell_tag()));
            }

            if (x < -threshold_)
            {
                Base::processOrder(Base::createOrder(volume_(), buy_tag()));
            }
        }

    private:
        VolumeDistr     volume_;
        double          threshold_;
    };
}

#endif