#ifndef marketsim_py_ref_counted_h_included
#define marketsim_py_ref_counted_h_included

#include <boost/python.hpp>

namespace marketsim
{
    namespace py = boost::python;

    template <class Base>
        struct PyRefCounted : Base
    {
        PyRefCounted() 
            :   ref_count(0)
            ,   pyobject(0)
        {
        }

        template <class T>
            PyRefCounted(T const &x)
                :   Base(x)
                ,   ref_count(0)
                ,   pyobject(0)
            {}

        void add_ref()
        {
            ++ref_count;
        }

        void release()
        {
            if (--ref_count == 0)
                delete this;
        }

        void set_pyobject(PyObject * pyobject)
        {
            this->pyobject = pyobject;
        }

        void clear_pyobject()
        {
            //std::cout << "[clearing pyobject pointer]" << std::endl;
            pyobject = 0;
        }

        PyObject * get_pyobject()
        {
            return pyobject;
        }

    protected:
        int         ref_count;
        PyObject *  pyobject;
    };

    template <class Base>
        struct PyRefCountedPersistent : Base
    {
        PyRefCountedPersistent(bool persistent) 
            :   persistent(persistent)
        {}

        template <class T>
        PyRefCountedPersistent(T const &x)
            :   Base(boost::get<0>(x))
            ,   persistent(boost::get<1>(x))
        {}

        void add_ref()
        {
            if (persistent)
            {
                if (pyobject && (ref_count > 0))
                {
                    py::incref(pyobject);
                }
            }
            Base::add_ref();
        }

        void release()
        {
            if (persistent)
            {
                if (pyobject)
                {
                    py::decref(pyobject);
                }
            }
            Base::release();
        }

        void set_pyobject(PyObject * pyobject)
        {
            if (persistent)
            {
                for (int i=0; i<ref_count-1; i++)
                    py::incref(pyobject);
            }
            Base::set_pyobject(pyobject);
        }

    private:
        bool persistent;
    };
}

namespace boost
{
    template <typename D> void intrusive_ptr_set_pyobject(marketsim::PyRefCounted<D>* ptr, PyObject * pyobject)  { ptr->set_pyobject(pyobject); }
    template <typename D> void intrusive_ptr_clear_pyobject(marketsim::PyRefCounted<D> * ptr) { ptr->clear_pyobject(); }
    template <typename D> PyObject * intrusive_ptr_get_pyobject(marketsim::PyRefCounted<D> * ptr)  { return ptr->get_pyobject(); }

    template <typename D> void intrusive_ptr_add_ref(marketsim::PyRefCounted<D> * p) { p->add_ref(); }
    template <typename D> void intrusive_ptr_release(marketsim::PyRefCounted<D> * p) { p->release(); }
}

#endif