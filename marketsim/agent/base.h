#ifndef _marketsim_agent_base_h_included_
#define _marketsim_agent_base_h_included_

#include <marketsim/common_types.h>

namespace marketsim
{
    template <class Derived>
        struct AgentBase 
    {
        AgentBase() {}
        AgentBase(Dummy) {}

        typedef Derived  derived_t;

        DECLARE_ARROW(Derived);

        template <class T>
            void onOrderFilled(T x) 
        {}

        template <class T>
            void onOrderPartiallyFilled(T x, PriceVolume const & y)
        {}

        template <class T>
            void onOrderCancelled(T x) 
        {}

		void getHandler() const; // to be defined in derived classes if used
    };
}

#endif