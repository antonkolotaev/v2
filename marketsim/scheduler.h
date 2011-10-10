#ifndef _marketsim_scheduler_h_included_
#define _marketsim_scheduler_h_included_

#include <queue>
#include <boost/intrusive_ptr.hpp>
#include <marketsim/ref_counted.h>

namespace marketsim 
{
    typedef double Time;
    typedef double TimeInterval;
    
    struct IEventHandler : RefCounted<IEventHandler>
    {
        virtual void process() = 0;

        // this function is non-virtual since it will be called very often 
        // and its value must not be changed while the event handler is in the event queue
        Time getActionTime() const 
        { 
            return actionTime_; 
        } 

        friend bool operator < (IEventHandler const & lhs, IEventHandler const & rhs)
        {
            return lhs.getActionTime() > rhs.getActionTime();
        }

    private:
        Time  actionTime_;
    protected:
        void setActionTime(Time t) { actionTime_ = t; }
        virtual void on_released() = 0;
        friend struct RefCounted<IEventHandler>;
    };

    typedef boost::intrusive_ptr<IEventHandler>     IEventHandlerPtr;

    struct IEventHandlerPtrCmp 
    {
        bool operator () (IEventHandlerPtr const & lhs, IEventHandlerPtr const & rhs)
        {
            return lhs->getActionTime() > rhs->getActionTime();
        }
    };

    template <typename IEventHandlerPtr, typename IEventHandlerPtrCmp>
        struct SchedulerT
            :  protected std::priority_queue<IEventHandlerPtr, std::vector<IEventHandlerPtr>, IEventHandlerPtrCmp >
    {
        // NOTE: another data structure with priority_queue interface might be chosen here
        typedef 
            std::priority_queue<IEventHandlerPtr, std::vector<IEventHandlerPtr>, IEventHandlerPtrCmp >
            Queue;

        SchedulerT() : currentTime_(0)
        {
            assert(instance_ == 0);
            instance_ = this;
        }

        ~SchedulerT()
        {
            assert(instance_ == this);
            instance_ = 0;
        }

        static Time currentTime() 
        {
            return instance_->currentTime_impl();
        }

        static void schedule(IEventHandlerPtr eh)
        {
            instance_->schedule_impl(eh);
        }

        static void cancel(IEventHandlerPtr eh)
        {
            // if there is no scheduler no events are scheduled
            if (instance_)
                instance_->cancel_impl(eh);
        }


        bool empty() const 
        {
            return Queue::empty();
        }


        void makeStep() 
        {
            IEventHandlerPtr  t = Queue::top();
            currentTime_ = t->getActionTime();
            Queue::pop();
            t->process();
        }

        void workTill(Time t)
        {
            while (!Queue::empty() && Queue::top()->getActionTime() < t)
            {
                makeStep();
            }
            currentTime_ = t;
        }

        void reset() 
        {
            while (!Queue::empty())
                Queue::pop();
            currentTime_ = 0;
        }

#ifdef MARKETSIM_BOOST_PYTHON

        static void py_register(const char * name)
        {
            using namespace boost::python;
            class_<SchedulerT, boost::noncopyable>(name)
                .def("workTill", &SchedulerT::workTill)
                .def("currentTime", &SchedulerT::currentTime_impl)
                .def("reset", &SchedulerT::reset)
                ;
        }

#endif 

    private:

        // O(nlogn)!!!
        void cancel_impl(IEventHandlerPtr eh)
        {
            c.erase(std::remove(c.begin(), c.end(), eh), c.end());
            std::make_heap(c.begin(), c.end(), comp);
        }

        void schedule_impl(IEventHandlerPtr eh)
        {
            Queue::push(eh);
        }

        Time currentTime_impl() const 
        {
            return currentTime_;
        }

        static SchedulerT*   instance_;

    private:
        Time     currentTime_;
    };

    typedef SchedulerT<IEventHandlerPtr, IEventHandlerPtrCmp> Scheduler;

	template <typename T, typename C> SchedulerT<T,C>* SchedulerT<T,C>::instance_ = 0;

    struct EventHandler : IEventHandler
    {
        void schedule(TimeInterval dt)
        {
            setActionTime(Scheduler::currentTime() + dt);
            Scheduler::schedule(this);
        }

        void cancel() 
        {
            Scheduler::cancel(this);
        }
    };

    template <typename T, typename DelayGenerator>
        struct Timer : EventHandler
    {
        typedef void (T::*Handler)();

        Timer(T & parent, Handler h, DelayGenerator d) 
            : parent_(parent), handler_(h), delay_(d)
        {
            schedule(delay_());
            /// TODO: add_ref for parent
        }

        void process()
        {
            (parent_.*handler_)();
            schedule(delay_());
        }

        void on_released() 
        {
            /// TODO: release for parent
        }

        ~Timer()
        {
            cancel();
        }

    private:
        T   &           parent_;
        Handler         handler_;
        DelayGenerator  delay_;
    };

    template <typename Derived, int ID = 0>
        struct EventHandlerEx : EventHandler
        {
            void process()
            {
                static_cast<Derived&>(*this).process(this);
            }
        };
}

#endif