#ifndef marketsim_py_ref_counted_h_included
#define marketsim_py_ref_counted_h_included

#include <boost/python.hpp>
#include <boost/tuple/tuple.hpp>
#include <marketsim/utils/intrusive_ptr_python_helper.h>
#include <iostream>

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
            std::cout << "[clearing pyobject pointer]" << std::endl;
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

    template <class T> 
        boost::intrusive_ptr<T> New_0()
    {
        return boost::intrusive_ptr<T>(new T());
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6> 
        boost::intrusive_ptr<T> New_6(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
    {
        return boost::intrusive_ptr<T>(new T(a1, a2, a3, a4, a5, a6));
    }

    template <class T>
        boost::python::object Init_0(boost::python::object py_obj)
    {
        using namespace boost::python;
        object return_value = call_method<object>(py_obj.ptr(), "__cons__");
        intrusive_ptr_set_pyobject(extract<T*>(py_obj), py_obj.ptr());
        return return_value;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
        boost::python::object Init_6(boost::python::object py_obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
    {
        using namespace boost::python;
        object return_value = call_method<object>(py_obj.ptr(), "__cons__", a1, a2, a3, a4, a5, a6);
        intrusive_ptr_set_pyobject(extract<T*>(py_obj), py_obj.ptr());
        return return_value;
    }

    template <class T>
        void register_pointer()
    {
        using namespace boost::python;
        converter::intrusive_ptr_from_python<T>();
        register_ptr_to_python< boost::intrusive_ptr<T> >();
    }

    template <class T, class ClassDef>
    void register_0(ClassDef &x)
    {
        x   .def("__init__", Init_0<T>)
            .def("__cons__", make_constructor(New_0<T>))
            .def("__del__",  &boost::intrusive_ptr_clear_pyobject);

        register_pointer<T>();
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class ClassDef>
    void register_6(ClassDef &x)
    {
        x   .def("__init__", Init_6<T,A1,A2,A3,A4,A5,A6>)
            .def("__cons__", make_constructor(New_6<T,A1,A2,A3,A4,A5,A6>))
            .def("__del__",  &boost::intrusive_ptr_clear_pyobject);

        register_pointer<T>();
    }

    template <typename T>
    void py_register(const char * name)
    {
        static bool registered = false;
        if (!registered) {
            T::py_register(name);
            registered = true;
        }
    }

    template <typename T>
    void py_register()
    {
        static bool registered = false;
        if (!registered) {
            T::py_register();
            registered = true;
        }
    }
}

#endif