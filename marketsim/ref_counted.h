#ifndef _marketsim_ref_counted_h_included_
#define _marketsim_ref_counted_h_included_

#include <boost/noncopyable.hpp>
#include <marketsim/common_types.h>

#ifdef MARKETSIM_BOOST_PYTHON
#  include <boost/python.hpp>
#endif

namespace marketsim
{
    struct IRefCounted : boost::noncopyable
    {
        virtual void add_ref() = 0;
        virtual void release() = 0;
#ifdef MARKETSIM_BOOST_PYTHON
        virtual void set_pyobject(PyObject *) = 0;
        virtual void clear_pyobject() = 0;
        virtual PyObject* get_pyobject() = 0;

        static void py_register()
        {
            using namespace boost::python;
            class_<IRefCounted, boost::intrusive_ptr<IRefCounted>, boost::noncopyable>
                ("RefCounted", no_init)
                ;
        }
#endif
        virtual ~IRefCounted() {}
    };

    template <class T>
        struct weak_intrusive_ptr
        {
            weak_intrusive_ptr(T *pointee)
                :   pointee(pointee)
                ,   prev(0)
            {
                weak_intrusive_ptr *old_head = pointee->get_weak_ptr_head();

                next = old_head;
                if (old_head)
                    old_head->prev = this;

                pointee->set_weak_ptr_head(this);
            }

            ~weak_intrusive_ptr()
            {
                if (next)
                    next->prev = prev;

                if (prev)
                    prev->next = next;

                if (pointee && pointee->get_weak_ptr_head()==this)
                {
                    pointee->set_weak_ptr_head(next);
                }
            }

            void reset()
            {
                assert(prev==0 || prev->pointee==0);
                pointee = 0;
                if (next)
                    next->reset();
            }

            T* operator -> () const 
            {
                return pointee;
            }

            T& operator * () const
            {
                return *pointee;
            }

            operator bool () const
            {
                return pointee != 0;
            }

        private:
            T                   *pointee;
            weak_intrusive_ptr  *prev, *next;
        };

        template <class Base>
            struct HasWeakReferences : Base 
        {
            HasWeakReferences() : head(0) {}
            HasWeakReferences(Dummy) : head(0) {}

            typedef typename Base::derived_t pointee_type; 

            void set_weak_ptr_head(weak_intrusive_ptr<pointee_type> * p)
            {
                head = p;
            }

            weak_intrusive_ptr<pointee_type>* get_weak_ptr_head() const 
            {
                return head;
            }

            ~HasWeakReferences()
            {
                if (head)
                    head->reset();
            }

        private:

            weak_intrusive_ptr<pointee_type>    *head;
        };

    //inline void intrusive_ptr_add_ref(IRefCounted * p) { p->add_ref(); }
    //inline void intrusive_ptr_release(IRefCounted * p) { p->release(); }


    /// base class for objects managed through boost::intrusive_ptr
    /// in order to make it as fast as possible we have no virtual functions here
    /// Derived class must define on_released() function with
    /// reaction on ref_counter == 0 (destroying itself)
    template <
        typename Base  
    >   
        struct RefCounted : boost::noncopyable, Base
    {
        RefCounted() : counter_(0) {}
        RefCounted(Dummy) : counter_(0) {}

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
                this->self()->on_released();
            }
        }

        int refCount() const { return counter_; }

        /*virtual*/~RefCounted() { assert(counter_ == 0); }

    private:
        int     counter_;
    };

}

namespace boost
{
    inline void intrusive_ptr_add_ref(marketsim::IRefCounted * ptr) { ptr->add_ref(); }
    inline void intrusive_ptr_release(marketsim::IRefCounted * ptr) { ptr->release(); }
#ifdef MARKETSIM_BOOST_PYTHON
    inline void intrusive_ptr_set_pyobject(marketsim::IRefCounted * ptr, PyObject * pyobject)  { ptr->set_pyobject(pyobject); }
    inline void intrusive_ptr_clear_pyobject(marketsim::IRefCounted * ptr) { ptr->clear_pyobject(); }
    inline PyObject * intrusive_ptr_get_pyobject(marketsim::IRefCounted * ptr)  { return ptr->get_pyobject(); }

    template <class T> void intrusive_ptr_clear_pyobject_t(T * ptr) { ptr->clear_pyobject(); }
#endif

    template <typename D> void intrusive_ptr_add_ref(marketsim::RefCounted<D> * p) { p->add_ref(); }
    template <typename D> void intrusive_ptr_release(marketsim::RefCounted<D> * p) { p->release(); }
}

#endif