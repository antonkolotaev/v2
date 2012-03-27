#ifndef _marketsim_fast_signal_h_included
#define _marketsim_fast_signal_h_included

#include <marketsim/agent/signal.h>

namespace marketsim {
namespace fast {

    namespace agent 
    {
        using namespace marketsim::agent;

        struct Signal : 
            marketsim::agent::Signal  < rng::exponential<Time>, rng::normal<double>, boost::intrusive_ptr<Signal_Trader>,
            PyRefCounted              < IRefCounted >
            >
        {
            Signal(boost::intrusive_ptr<Signal_Trader>  trader, Time meanUpdateTime, double deltaSigma)
                :   base(rng::exponential<Time>(meanUpdateTime), rng::normal<double>(0., deltaSigma), trader)
            {}

            static void py_register(const char * name)
            {
                using namespace boost::python;

                class_<
                    Signal, 
                    boost::intrusive_ptr<Signal>,
                    boost::noncopyable
                > 
                c(name, no_init);

                base::py_visit(c);
                register_3<Signal, boost::intrusive_ptr<Signal_Trader>, Time, double>(c);
            }
        };
    }
}}

MARKETSIM_PY_REGISTER_NAME(marketsim::fast::agent::Signal, "Signal");

#endif