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

    struct Scheduler
    {
        void schedule(IEventHandler* eh)
        {
            eventQueue_.push(eh);
        }

        bool empty() const 
        {
            return eventQueue_.empty();
        }

        Time currentTime() const 
        {
            return currentTime_;
        }

        void makeStep() 
        {
            IEventHandlerPtr  t = eventQueue_.top();
            currentTime_ = t->getActionTime();
            eventQueue_.pop();
            t->process();
        }

        Scheduler() : currentTime_(0) {}

        void workTill(Time t)
        {
            while (!eventQueue_.empty() && eventQueue_.top()->getActionTime() < t)
            {
                makeStep();
            }
            currentTime_ = t;
        }

        struct Session
        {
            ~Session();
        };

        void reset() 
        {
            while (!eventQueue_.empty())
                eventQueue_.pop();
            currentTime_ = 0;
        }

    private:
        // NOTE: another data structure with priority_queue interface might be chosen here
        typedef 
            std::priority_queue<IEventHandlerPtr, std::vector<IEventHandlerPtr>, IEventHandlerPtrCmp >
            Queue;

        Queue    eventQueue_;
        Time     currentTime_;
    };

    inline Scheduler& scheduler() 
    {
        static Scheduler instance;
        return instance;   
    }

    inline Scheduler::Session::~Session()
    {
        scheduler().reset();
    }

    struct EventHandler : IEventHandler
    {
        void schedule(TimeInterval dt)
        {
            setActionTime(scheduler().currentTime() + dt);
            scheduler().schedule(this);
        }
    };

    template <class T, class DelayGenerator>
        struct Timer : EventHandler
    {
        typedef void (T::*Handler)();

        Timer(T & parent, Handler h, DelayGenerator d) 
            : parent_(parent), handler_(h), delay_(d)
        {
            schedule(delay_());
        }

        void process()
        {
            (parent_.*handler_)();
            schedule(delay_());
        }

        void on_released() 
        {}

    private:
        T   &           parent_;
        Handler         handler_;
        DelayGenerator  delay_;
    };

    template <class Derived, int ID = 0>
        struct EventHandlerEx : EventHandler
        {
            void process()
            {
                static_cast<Derived&>(*this).process(this);
            }
        };
}

#endif