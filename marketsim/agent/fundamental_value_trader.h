#ifndef _marketsim_agent_fundamental_value_trader_h_included_
#define _marketsim_agent_fundamental_value_trader_h_included_

#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim 
{
    template <typename IntervalDistr, typename VolumeDistr, typename Base>
        struct FundamentalValueTrader : Base 
    {
        typedef FundamentalValueTrader   base; // for derived typenamees

        template <typename T>
        FundamentalValueTrader(T const & x)
            :   Base    (boost::get<0>(x))
            ,   timer_  (*this, &FundamentalValueTrader::createOrder, boost::get<1>(x))
            ,   volume_           (boost::get<2>(x))
            ,   fundamentalValue2_(boost::get<3>(x) * 2)
        {}

        DECLARE_BASE(FundamentalValueTrader);

    private:
        void createOrder()
        {
            // TODO: consider trading if one side is empty
            if (!Base::getOrderBook()->empty<Sell>() && !Base::getOrderBook()->empty<Buy>())
            {
                Price midPrice2 = 
                    Base::getOrderBook()->bestPrice<Sell>() + 
                    base::getOrderBook()->bestPrice<Buy>();

                Price delta = midPrice2 - fundamentalValue2_;
                Volume v = (Volume)volume_();

                if (delta < 0)
                    self()->sendBuyOrder(v);

                if (delta > 0)
                    self()->sendSellOrder(v);
            }
        }

    private:
        typedef Timer<FundamentalValueTrader, IntervalDistr> timer_t;

        timer_t       timer_;
        VolumeDistr   volume_;
        Price const   fundamentalValue2_;
    };
}

#endif