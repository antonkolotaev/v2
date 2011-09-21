#ifndef _marketsim_agent_liquidity_provider_h_included_
#define _marketsim_agent_liquidity_provider_h_included_

#include <marketsim/object_pool.h>
#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim
{
    template <class GenerationPeriod, class LagDistribution, class VolumeDistribution, class Base>
        struct LiquidityProvider : Base
    {
        template <class T> LiquidityProvider(T const & x) 
            : Base   (boost::get<0>(x))
            , timer_ (*self(), &LiquidityProvider::createOrder, boost::get<1>(x))
            , lag_   (boost::get<2>(x))
            , volume_(boost::get<3>(x))
            , initialPrice_(boost::get<4>(x)) 
        {}

        typedef typename Base::order_type order_type;

        typedef typename order_side<order_type> :: type  side;

        typedef LiquidityProvider   base;  // for derived classes

        void createOrder()
        {
            Price tickSize = getOrderBook()->getTickSize();
            Price bp = getOrderBook()->empty(side()) ? initialPrice_ : getOrderBook()->bestPrice(side());
            Price delta =   (Price)(lag_() / tickSize) * tickSize /*+ 1*/;
            Price p = std::max(Price(0), side::value == Sell ? bp + delta : bp - delta);
            
            Volume v = std::max(Volume(1), volume_());

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