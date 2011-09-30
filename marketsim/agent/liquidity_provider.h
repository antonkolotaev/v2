#ifndef _marketsim_agent_liquidity_provider_h_included_
#define _marketsim_agent_liquidity_provider_h_included_

#include <marketsim/object_pool.h>
#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

#undef max

namespace marketsim
{
    template <typename GenerationPeriod, typename LagDistribution, typename VolumeDistribution, typename Base>
        struct LiquidityProvider : Base
    {
        template <typename T> LiquidityProvider(T const & x) 
            : Base   (boost::get<0>(x))
            , timer_ (*self(), &LiquidityProvider::sendOrder, boost::get<1>(x))
            , lag_   (boost::get<2>(x))
            , volume_(boost::get<3>(x))
            , initialPrice_(boost::get<4>(x)) 
        {}

        typedef typename Base::order_type order_type;

        typedef typename order_side<order_type> :: type  side;

        DECLARE_BASE(LiquidityProvider)

        void sendOrder()
        {
            Price bp = getOrderBook()->empty(side()) ? initialPrice_ : getOrderBook()->bestPrice(side());
            Price delta = getOrderBook()->ceilPrice(lag_());
            Price p = std::max(Price(1), side::value == Sell ? bp + delta : bp - delta);
            
            Volume v = std::max(Volume(1), static_cast<Volume>(volume_()));

            Base::processOrder(Base::createOrder(pv(p,v)));
        }

    private:
        typedef Timer<LiquidityProvider, GenerationPeriod>  timer_t;

        timer_t             timer_;
        LagDistribution     lag_;
        VolumeDistribution  volume_;
        Price const         initialPrice_;
    };

}

#endif