#ifndef _marketsim_agent_base_h_included_
#define _marketsim_agent_base_h_included_

#include <marketsim/common_types.h>

namespace marketsim {
namespace agent {
    /// The most base class for agents
    template <typename Derived, class Base = Empty>
    struct AgentBase : Base
    {
        /// no meaningful parameters should be passed here
        AgentBase() {}
        AgentBase(Dummy) {}
        AgentBase(boost::tuples::tuple<>) {}

        typedef Derived  derived_t;

        DECLARE_ARROW(Derived);

        //------------------- default implementation for handlers for events from orders
        template <typename T>
            void onOrderFilled(T x) 
        {}

        template <typename T>
            void onOrderPartiallyFilled(T x, PriceVolume const & y)
        {}

        template <typename T>
            void onOrderCancelled(T x) 
        {}

		void getHandler() const; // to be defined in derived classes if used

#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T> 
            static void py_visit(T & class_def)
            {}
#endif
    };
}}

#endif