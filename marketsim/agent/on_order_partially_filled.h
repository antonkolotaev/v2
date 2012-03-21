#ifndef _marketsim_agent_on_order_partially_filled_h_included_
#define _marketsim_agent_on_order_partially_filled_h_included_

#include <marketsim/common_types.h>

namespace marketsim {
namespace agent 
{
    /// Base class for agents 
    /// Defines an extension point for handling order's onOrderPartiallyFilled event
    /// Usually it can be used for recording some history about the agent
    template <
        typename Handler,       // functor-like type defining operator () (AgentType*)
        typename Base
    >
        struct OnPartiallyFilled : Base
    {
        /// 0-th argument is passed to the base class
        /// 1-th argument is initializer for Handler
        template <typename T>
            OnPartiallyFilled(T const & x)
				:   Base   (boost::get<0>(x)) 
				,	handler_(boost::get<1>(x))
			{}

        DECLARE_BASE(OnPartiallyFilled);

        /// Grants access to the handler
        /// Since there might be several handlers in an agent
        /// we use a tag type (Handler*) to differentiate between them
        Handler         & getHandler(Handler*)       { return handler_; }
        Handler const   & getHandler(Handler*) const { return handler_; }

        using Base::getHandler;

        /// calls Handler 
        template <typename Order>
            void onOrderPartiallyFilled(Order order, PriceVolume const & x)
        {
            Base::onOrderPartiallyFilled(order, x);
            handler_(this->self());            
        }

#ifdef MARKETSIM_BOOST_PYTHON
        
        template <class T>
            static void py_visit(T & c)
            {
                Base::py_visit(c);
                c.def_readonly("on_partially_filled", &OnPartiallyFilled::handler_);
            }

#endif

    private:
        Handler     handler_;
    };

}}

#endif
