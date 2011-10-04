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

#ifdef MARKETSIM_BOOST_PYTHON
        
        template <class T>
            static void py_visit(T & c)
            {
                Base::py_visit(c);
                c.def_readonly("on_partially_filled", &OnPartiallyFilled::Handler_);
            }

#endif

    //private:
    protected:
        Handler     Handler_;
    };

}

#endif