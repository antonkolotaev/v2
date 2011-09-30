#ifndef _marketsim_agent_signal_h_included_
#define _marketsim_agent_signal_h_included_

#include <marketsim/scheduler.h>

namespace marketsim
{
    template <typename IntervalDistr, typename DeltaDistr, typename ListenerPtr>
        struct Signal 
    {
        Signal(IntervalDistr t, DeltaDistr d, ListenerPtr l)
            :   timer_(*this, &Signal::notify, t)
            ,   delta_(d)
            ,   listener_(l)
            ,   value_(0)
        {}

        DECLARE_BASE(Signal);

        void notify()
        {
            listener_->onSignal(value_ += delta_());   
        }

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
        typedef Timer<Signal, IntervalDistr>   timer_t;

        timer_t         timer_;
        DeltaDistr      delta_;
        ListenerPtr     listener_;
        double          value_;
    };

}

#endif