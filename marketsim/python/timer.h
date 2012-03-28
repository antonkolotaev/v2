#ifndef _marketsim_python_timer_h_included
#define _marketsim_python_timer_h_included

#include <marketsim/python/py_ref_counted.h>
#include <marketsim/scheduler.h>

namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

    struct Timer : PyRefCounted<EventHandlerBase>
    {
        Timer(boost::python::object intervals, boost::python::object handler)
            :   handler  (handler)
            ,   intervals(intervals)
        {
            schedule(this->intervals());
        }

        void process()
        {
            boost::python::object res = handler();
            try {
                bool cont = extract<bool>(res);
                if (!cont)
                    return;
            } catch (...) {}
            schedule(intervals()); 
        }

        static void py_register()
        {
            using namespace boost::python;

            class_<
                Timer, 
                boost::intrusive_ptr<Timer>,
                boost::noncopyable
            > c("Timer", no_init);

            c.def("cancel", &Timer::cancel);

            register_2<Timer, object, object>(c);
        }
        

        boost::python::object   handler;
        py_value<TimeInterval>  intervals;
    };

}}
#endif