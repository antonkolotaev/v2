#ifndef _marketsim_agent_on_order_partially_filled_h_included_
#define _marketsim_agent_on_order_partially_filled_h_included_

namespace marketsim
{
    template <class Handler, class Base>
        struct OnPartiallyFilled : Base
    {
        template <class T>
            OnPartiallyFilled(T const & x)
				:   Base    (boost::get<0>(x)) 
				,	Handler_(boost::get<1>(x))
			{}

        typedef OnPartiallyFilled base; // for derived classes

		Handler & getHandler(Handler*) { return Handler_; }
		Handler const & getHandler(Handler*) const { return Handler_; }

		using Base::getHandler;

        template <class Order>
            void onOrderPartiallyFilled(Order order, PriceVolume const & x)
        {
            Base::onOrderPartiallyFilled(order, x);
            Handler_(self());            
        }
    private:
        Handler     Handler_;
    };

}

#endif