#ifndef _marketsim_agent_signal_h_included_
#define _marketsim_agent_signal_h_included_

#include <marketsim/scheduler.h>

namespace marketsim {
namespace agent 
{
    /// Class that generates a signal in some moments of time
    /// TODO: inversion of control: a signal trader is parametrized by signal and subscribes to onSignal
    template <
        typename IntervalDistr,     // generator of intervals between signal value updates
        typename DeltaDistr,        // generator of deltas for the signal value
        typename ListenerPtr        // a pointer to a class with onSignal function
    >
        struct Signal 
    {
        /// 0-th argument defines intervals between signal updates
        /// 1-th argument is a generator for signal deltas
        /// 2-th argument is a listener of signal value change events
        Signal(IntervalDistr t, DeltaDistr d, ListenerPtr l)
            :   timer_   (*this, &Signal::notify, t)
            ,   delta_   (d)
            ,   listener_(l)
            ,   value_   (0)
        {}

        DECLARE_BASE(Signal);

        /// \return current signal value
        double getValue() const 
        {
            return value_;
        }
#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T>
            static void py_visit(T & c)
            {
                c.def_readwrite("signalValue", &Signal::value_);
            }
#endif
    private:
        void notify()
        {
            listener_->onSignal(value_ += delta_());   
        }

    private:
        typedef Timer<Signal, IntervalDistr>   timer_t;

        timer_t         timer_;
        DeltaDistr      delta_;
        ListenerPtr     listener_;
        double          value_;
    };

}}

#endif