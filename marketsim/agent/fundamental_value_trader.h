#ifndef _marketsim_agent_fundamental_value_trader_h_included_
#define _marketsim_agent_fundamental_value_trader_h_included_

#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim 
{
    /// This base class for agents encapsulates a logic for fundamental value trading
    /// In moments of times defined by distribution IntervalDistr
    /// it estimates middle price in the market, generates using VolumeDistr an amount to be traded
    /// and if current mid price is below than given fundamental value sends a buy order
    /// if current mid price is higher than given fundamental value sends a sell order
    template <typename IntervalDistr, typename VolumeDistr, typename Base>
        struct FundamentalValueTrader : Base 
    {
        /// 0-th argument is passed to the base class
        /// 1-th argument defines interval distribution between order generation
        /// 2-th argument defines order size distribution
        /// 3-th argument defines The Fundamental Value
        template <typename T>
            FundamentalValueTrader(T const & x)
            :   Base                (boost::get<0>(x))
            ,   timer_              (*this, &FundamentalValueTrader::createOrder, boost::get<1>(x))
            ,   volume_             (boost::get<2>(x))
            ,   fundamentalValue2_  (boost::get<3>(x) * 2)
        {}

        DECLARE_BASE(FundamentalValueTrader);

    private:
        void createOrder()
        {
            Volume v = (Volume)volume_();

            if (!Base::getOrderBook()->empty<Sell>() && !Base::getOrderBook()->empty<Buy>())
            {
                Price midPrice2 = 
                    Base::getOrderBook()->bestPrice<Sell>() + 
                    base::getOrderBook()->bestPrice<Buy>();

                Price delta = midPrice2 - fundamentalValue2_;

                if (delta < 0)
                    self()->sendMarketOrder<Buy>(v);

                if (delta > 0)
                    self()->sendMarketOrder<Sell>(v);
            } 
            else if (!Base::getOrderBook()->empty<Sell>())
            {
                if (Base::getOrderBook()->bestPrice<Sell>()*2 < fundamentalValue2_)
                {
                    self()->sendMarketOrder<Buy>(v);
                }
            } 
            else if (!Base::getOrderBook()->empty<Buy>())
            {
                if (Base::getOrderBook()->bestPrice<Buy>()*2 > fundamentalValue2_)
                {
                    self()->sendMarketOrder<Sell>(v);
                }
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