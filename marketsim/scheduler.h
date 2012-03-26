#ifndef _marketsim_scheduler_h_included_
#define _marketsim_scheduler_h_included_

#include <queue>
#include <boost/intrusive_ptr.hpp>
#include <marketsim/ref_counted.h>
#include <marketsim/common/macros.h>

#ifdef MARKETSIM_BOOST_PYTHON
#  include <marketsim/py_ref_counted.h>
#endif

namespace marketsim 
{
    typedef double Time;
    // Time - Time == TimeInterval
    typedef double TimeInterval;
    
    /// Base class for events which can be scheduled
    struct IEventHandler : IRefCounted
    {
        IEventHandler() : cancelled_(false) {}

        /// method to be called when time for the event come
        virtual void process() = 0;

        bool cancelled() const { return cancelled_; }

        void cancel() { cancelled_ = true; }

        // this function is non-virtual since it will be called very often 
        // its value must not be changed while the event handler is in the event queue
        Time getActionTime() const 
        { 
            return actionTime_; 
        } 

        /// compares two events by its time
        friend bool operator < (IEventHandler const & lhs, IEventHandler const & rhs)
        {
            return lhs.getActionTime() > rhs.getActionTime();
        }

#ifdef MARKETSIM_BOOST_PYTHON
        void set_pyobject(PyObject *) { assert(0); }
        void clear_pyobject() { assert(0); }
        PyObject* get_pyobject() { assert(0); return 0; }
#endif

    private:
        Time  actionTime_;
        bool  cancelled_;
    protected:
        /// changes action time for the event
        void setActionTime(Time t) { actionTime_ = t; }
        /// called when the event is released
        ///virtual void on_released() = 0;
        ///friend struct RefCounted<derived_is<IEventHandler> >;
    };

    /// by default, we use boost::intrusive_ptr to store events
    typedef boost::intrusive_ptr<IEventHandler>     IEventHandlerPtr;

    /// Compares events by their action time
    struct IEventHandlerPtrCmp 
    {
        bool operator () (IEventHandlerPtr const & lhs, IEventHandlerPtr const & rhs)
        {
            return lhs->getActionTime() > rhs->getActionTime();
        }
    };

    /// Scheduler is the core of a simulation
    /// It controls model time and manages an ordered by time set of events
    /// Scheduler was made a singleton for convenience 
    template <
        typename IEventHandlerPtr,      /// a smart pointer to event: Time getActionTime(); void process();
        typename IEventHandlerPtrCmp    /// compares IEventHandlerPtr by their action time
    >
        struct SchedulerT
            :  protected std::priority_queue<IEventHandlerPtr, std::vector<IEventHandlerPtr>, IEventHandlerPtrCmp >
#ifdef MARKETSIM_BOOST_PYTHON
            ,   PyRefCounted<IRefCounted>
#endif
    {
        // NOTE: another data structure with priority_queue interface might be chosen here
        typedef 
            std::priority_queue<IEventHandlerPtr, std::vector<IEventHandlerPtr>, IEventHandlerPtrCmp >
            Queue;

        /// constructs a scheduler; now its sigleton refers to this instance
        SchedulerT() : currentTime_(0)
        {
            assert(instance_ == 0);
            instance_ = this;
        }

        /// destructs the scheduler; its singleton refers to 0
        ~SchedulerT()
        {
            assert(instance_ == this);
            instance_ = 0;
        }

        /// \return current model time 
        static Time currentTime() 
        {
            return instance_->currentTime_impl();
        }

        /// schedules an event into the queue
        static void schedule(IEventHandlerPtr eh)
        {
            instance_->schedule_impl(eh);
        }

        /// cancels an event. 
        static void onCancelled()
        {
            // if there is no scheduler no events are scheduled
            if (instance_)
                instance_->makeValid();
        }

        /// \return true iff there are no events
        bool empty() const 
        {
            return Queue::empty();
        }
    private:
        void makeValid()
        {
            while (!Queue::empty() && Queue::top()->cancelled())
            {
                Queue::pop();
            }
        }

        void makeStep_impl()
        {
            // taking an event with the least time
            IEventHandlerPtr  t = Queue::top();
            // updating current model time
            currentTime_ = t->getActionTime();
            // removing the event from the queue
            Queue::pop();
            // launching the event handler
            t->process();
        }
    public:

        /// makes a single simulation step
        void makeStep() 
        {
            makeValid();

            if (!Queue::empty())
            {
                makeStep_impl();
            }
        }

        /// advances the simulation while the model time < t
        void workTill(Time t)
        {
            assert(t >= currentTime_);
            /// while there are event with action time less than t
            while (!Queue::empty() && Queue::top()->getActionTime() < t)
            {
                if (Queue::top()->cancelled())
                    Queue::pop();
                else
                    /// launch them
                    makeStep_impl();
            }
            currentTime_ = t;
        }

        /// advances model time by dt
        void advance(TimeInterval dt)
        {
            workTill(currentTime() + dt);
        }

        /// restarts the scheduler (nb! O(nlogn))
        void reset() 
        {
            while (!Queue::empty()) Queue::pop();
            currentTime_ = 0;
        }

#ifdef MARKETSIM_BOOST_PYTHON

        static void py_register(const char * name)
        {
            using namespace boost::python;
            class_<SchedulerT, boost::intrusive_ptr<SchedulerT>, bases<IRefCounted>, boost::noncopyable> c(name, no_init);

            c   .def("workTill", &SchedulerT::workTill)
                .def("currentTime", &SchedulerT::currentTime_impl)
                .def("advance", &SchedulerT::advance)
                .def("reset", &SchedulerT::reset)
                ;

            register_0<SchedulerT>(c);
        }

#endif 

    private:
    public:
        /// schedules an event
        void schedule_impl(IEventHandlerPtr eh)
        {
            assert(currentTime_ <= eh->getActionTime());
            Queue::push(eh);
        }

        /// \return current model time
        Time currentTime_impl() const 
        {
            return currentTime_;
        }

        /// pointer to the only instance of the scheduler if exists
        static SchedulerT*   instance_;

    private:
        Time     currentTime_;
    };

    template <typename T, typename C> SchedulerT<T,C>* SchedulerT<T,C>::instance_ = 0;

    /// we will use a scheduler with events managed by boost::intrusive_ptr
    typedef SchedulerT<IEventHandlerPtr, IEventHandlerPtrCmp> Scheduler;

    /// standard base class class for event handlers
    struct EventHandlerBase : IEventHandler
    {
        /// schedules this class to be fired in dt 
        /// (so at currentTime + dt method process will be called)
        void schedule(TimeInterval dt)
        {
            setActionTime(Scheduler::currentTime() + dt);
            Scheduler::schedule(this);
        }

        /// cancels this event. warning: O(nlogn)
        void cancel() 
        {
            IEventHandler::cancel();
            Scheduler::onCancelled();
        }
    };

    template <
        typename T             // type which method will be called
    >
    struct TimeOut : EventHandlerBase
    {       
        typedef void (T::*Handler)();

        /// \param parent reference to object which method will be called
        /// \param h pointer to method to be called
        /// \param d functor returning time intervals
        TimeOut(T & parent, Handler h, TimeInterval dt) 
            : parent_(parent), handler_(h)
        {
            schedule(dt);
        }

        void add_ref()
        {
            parent_.add_ref();
        }

        void release()
        {
            parent_.release();
        }

        /// called from IEventHandler
        void process()
        {
            (parent_.*handler_)();
        }

        void on_released() 
        {
            /// TODO: release for parent
        }

        /// removes itself from the scheduler
        ~TimeOut()
        {
            cancel();
        }

    private:
        T   &           parent_;
        Handler         handler_;
    };


    /// wakes up in intervals defined by DelayGenerator and calls some T's method
    template <
        typename T,             // type which method will be called
        typename DelayGenerator // generates time intervals
    >
        struct Timer : EventHandlerBase
    {       
        typedef void (T::*Handler)();

        /// \param parent reference to object which method will be called
        /// \param h pointer to method to be called
        /// \param d functor returning time intervals
        Timer(T & parent, Handler h, DelayGenerator d) 
            : parent_(parent), handler_(h), delay_(d)
        {
            schedule(delay_());
        }

        void add_ref()
        {
            parent_.add_ref();
        }

        void release()
        {
            parent_.release();
        }

        /// called from IEventHandler
        void process()
        {
            (parent_.*handler_)();
            schedule(delay_());
        }

        void on_released() 
        {
            /// TODO: release for parent
        }

        /// removes itself from the scheduler
        ~Timer()
        {
            cancel();
        }

    private:
        T   &           parent_;
        Handler         handler_;
        DelayGenerator  delay_;
    };
}

#endif
