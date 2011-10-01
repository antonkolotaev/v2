#ifndef _marketsim_agent_on_order_partially_filled_h_included_
#define _marketsim_agent_on_order_partially_filled_h_included_

#include <marketsim/common_types.h>

namespace marketsim
{
    template <typename Handler, typename Base>
        struct OnPartiallyFilled : Base
    {
        template <typename T>
            OnPartiallyFilled(T const & x)
				:   Base    (boost::get<0>(x)) 
				,	Handler_(boost::get<1>(x))
			{}

        DECLARE_BASE(OnPartiallyFilled);

		Handler & getHandler(Handler*) { return Handler_; }
		Handler const & getHandler(Handler*) const { return Handler_; }

		using Base::getHandler;

        template <typename Order>
            void onOrderPartiallyFilled(Order order, PriceVolume const & x)
        {
            Base::onOrderPartiallyFilled(order, x);
            Handler_(self());            
        }
    //private:
    protected:
        Handler     Handler_;
    };

}

#endif