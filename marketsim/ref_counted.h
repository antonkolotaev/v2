#ifndef _marketsim_ref_counted_h_included_
#define _marketsim_ref_counted_h_included_

#include <boost/noncopyable.hpp>

namespace marketsim
{
    template <class Derived>   
        struct RefCounted : boost::noncopyable
    {
        RefCounted() : counter_(0) {}

        void add_ref() 
        {
            ++counter_; 
        }

        void release() 
        {
            if (--counter_ == 0)
            {
                static_cast<Derived*>(this)->on_released();
            }
        }

        int refCount() const { return counter_; }

        ~RefCounted() { assert(counter_ == 0); }

    private:
        int     counter_;
    };

    template <class D> void intrusive_ptr_add_ref(RefCounted<D> * p) { p->add_ref(); }
    template <class D> void intrusive_ptr_release(RefCounted<D> * p) { p->release(); }
}

#endif