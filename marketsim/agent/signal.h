#ifndef _marketsim_agent_signal_h_included_
#define _marketsim_agent_signal_h_included_

#include <marketsim/scheduler.h>

namespace marketsim
{
    template <class IntervalDistr, class DeltaDistr, class ListenerPtr>
        struct Signal 
    {
        Signal(IntervalDistr t, DeltaDistr d, ListenerPtr l)
            :   timer_(*this, &Signal::notify, t)
            ,   delta_(d)
            ,   listener_(l)
            ,   value_(0)
        {}

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

}

#endif