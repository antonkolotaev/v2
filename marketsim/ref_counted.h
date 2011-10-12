#ifndef _marketsim_ref_counted_h_included_
#define _marketsim_ref_counted_h_included_

#include <boost/noncopyable.hpp>

namespace marketsim
{
    /// base class for objects managed through boost::intrusive_ptr
    /// in order to make it as fast as possible we have no virtual functions here
    /// Derived class must define on_released() function with
    /// reaction on ref_counter == 0 (destroying itself)
    template <
        typename Derived  // class defining on_released function
    >   
        struct RefCounted : boost::noncopyable
    {
        RefCounted() : counter_(0) {}

        /// increments reference counter
        void add_ref() 
        {
            ++counter_; 
        }

        /// decrements reference counter
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

    template <typename D> void intrusive_ptr_add_ref(RefCounted<D> * p) { p->add_ref(); }
    template <typename D> void intrusive_ptr_release(RefCounted<D> * p) { p->release(); }
}

#endif