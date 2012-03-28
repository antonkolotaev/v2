#ifndef _marketsim_python_scheduled_event_h_included
#define _marketsim_python_scheduled_event_h_included

#include <marketsim/python/py_ref_counted.h>
#include <marketsim/scheduler.h>

namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

    struct ScheduledEvent : PyRefCounted<EventHandlerBase>
    {
        ScheduledEvent(TimeInterval dt, boost::python::object handler) 
            : handler(handler)
        {
            schedule(dt);
        }

        void process()
        {
            handler();
        }

        boost::python::object  handler;
        
        static void py_register()
        {
            using namespace boost::python;
            
            class_<
                ScheduledEvent, 
                boost::intrusive_ptr<ScheduledEvent>,
                boost::noncopyable
            > c("Event", no_init);

            c.def("cancel", &ScheduledEvent::cancel);

            register_2<ScheduledEvent, TimeInterval, object>(c);
        }
    };
}}

#endif