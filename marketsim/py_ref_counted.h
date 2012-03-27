#ifndef marketsim_py_ref_counted_h_included
#define marketsim_py_ref_counted_h_included

#include <boost/python.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <marketsim/utils/intrusive_ptr_python_helper.h>
#include <iostream>
#include <list>

#define MARKETSIM_HAS_REFERENCE_TO_PYTHON(X)\
namespace boost { namespace python  \
{                                   \
    template <>                     \
    struct has_back_reference<X>    \
        : mpl::true_                \
    {};                             \
}}

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

    template <class T, class A1> 
        boost::intrusive_ptr<T> New_1(A1 a1)
    {
        return boost::intrusive_ptr<T>(new T(a1));
    }

    template <class T, class A1, class A2> 
        boost::intrusive_ptr<T> New_2(A1 a1, A2 a2)
    {
        return boost::intrusive_ptr<T>(new T(a1, a2));
    }

    template <class T, class A1, class A2, class A3> 
        boost::intrusive_ptr<T> New_3(A1 a1, A2 a2, A3 a3)
    {
        return boost::intrusive_ptr<T>(new T(a1, a2, a3));
    }

    template <class T, class A1, class A2, class A3, class A4> 
        boost::intrusive_ptr<T> New_4(A1 a1, A2 a2, A3 a3, A4 a4)
    {
        return boost::intrusive_ptr<T>(new T(a1, a2, a3, a4));
    }

    template <class T, class A1, class A2, class A3, class A4, class A5> 
        boost::intrusive_ptr<T> New_5(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
    {
        return boost::intrusive_ptr<T>(new T(a1, a2, a3, a4, a5));
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

    template <class T, class A1>
        boost::python::object Init_1(boost::python::object py_obj, A1 a1)
    {
        using namespace boost::python;
        object return_value = call_method<object>(py_obj.ptr(), "__cons__", a1);
        intrusive_ptr_set_pyobject(extract<T*>(py_obj), py_obj.ptr());
        return return_value;
    }


    template <class T, class A1, class A2>
        boost::python::object Init_2(boost::python::object py_obj, A1 a1, A2 a2)
    {
        using namespace boost::python;
        object return_value = call_method<object>(py_obj.ptr(), "__cons__", a1, a2);
        intrusive_ptr_set_pyobject(extract<T*>(py_obj), py_obj.ptr());
        return return_value;
    }

    template <class T, class A1, class A2, class A3>
        boost::python::object Init_3(boost::python::object py_obj, A1 a1, A2 a2, A3 a3)
    {
        using namespace boost::python;
        object return_value = call_method<object>(py_obj.ptr(), "__cons__", a1, a2, a3);
        intrusive_ptr_set_pyobject(extract<T*>(py_obj), py_obj.ptr());
        return return_value;
    }

    template <class T, class A1, class A2, class A3, class A4>
        boost::python::object Init_4(boost::python::object py_obj, A1 a1, A2 a2, A3 a3, A4 a4)
    {
        using namespace boost::python;
        object return_value = call_method<object>(py_obj.ptr(), "__cons__", a1, a2, a3, a4);
        intrusive_ptr_set_pyobject(extract<T*>(py_obj), py_obj.ptr());
        return return_value;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5>
        boost::python::object Init_5(boost::python::object py_obj, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
    {
        using namespace boost::python;
        object return_value = call_method<object>(py_obj.ptr(), "__cons__", a1, a2, a3, a4, a5);
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
        using namespace boost::python;
        x   .def("__init__", Init_0<T>)
            .def("__cons__", make_constructor(New_0<T>))
            .def("__del__",  &boost::intrusive_ptr_clear_pyobject_t<T>);

        register_pointer<T>();
    }

    template <class T, class A1, class ClassDef>
    void register_1(ClassDef &x)
    {
        using namespace boost::python;
        x   .def("__init__", Init_1<T,A1>)
            .def("__cons__", make_constructor(New_1<T,A1>))
            .def("__del__",  &boost::intrusive_ptr_clear_pyobject_t<T>);

        register_pointer<T>();
    }

    template <class T, class A1, class A2, class ClassDef>
    void register_2(ClassDef &x)
    {
        using namespace boost::python;
        x   .def("__init__", Init_2<T,A1,A2>)
            .def("__cons__", make_constructor(New_2<T,A1,A2>))
            .def("__del__",  &boost::intrusive_ptr_clear_pyobject_t<T>);

        register_pointer<T>();
    }

    template <class T, class A1, class A2, class A3, class ClassDef>
    void register_3(ClassDef &x)
    {
        using namespace boost::python;
        x   .def("__init__", Init_3<T,A1,A2,A3>)
            .def("__cons__", make_constructor(New_3<T,A1,A2,A3>))
            .def("__del__",  &boost::intrusive_ptr_clear_pyobject_t<T>);

        register_pointer<T>();
    }

    template <class T, class A1, class A2, class A3, class A4, class ClassDef>
    void register_4(ClassDef &x)
    {
        using namespace boost::python;
        x   .def("__init__", Init_4<T,A1,A2,A3,A4>)
            .def("__cons__", make_constructor(New_4<T,A1,A2,A3,A4>))
            .def("__del__",  &boost::intrusive_ptr_clear_pyobject_t<T>);

        register_pointer<T>();
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class ClassDef>
    void register_5(ClassDef &x)
    {
        using namespace boost::python;
        x   .def("__init__", Init_5<T,A1,A2,A3,A4,A5>)
            .def("__cons__", make_constructor(New_5<T,A1,A2,A3,A4,A5>))
            .def("__del__",  &boost::intrusive_ptr_clear_pyobject_t<T>);

        register_pointer<T>();
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class ClassDef>
    void register_6(ClassDef &x)
    {
        using namespace boost::python;
        x   .def("__init__", Init_6<T,A1,A2,A3,A4,A5,A6>)
            .def("__cons__", make_constructor(New_6<T,A1,A2,A3,A4,A5,A6>))
            .def("__del__",  &boost::intrusive_ptr_clear_pyobject_t<T>);

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

    typedef std::list<boost::function<void ()> >  PyRegFunctions;

    PyRegFunctions& pyRegFunctions()
    {
        static PyRegFunctions  f;
        return f;
    }

    template <class T>
    struct RegisterInPython 
    {
        RegisterInPython()
        {
            pyRegFunctions().push_back(reg_0);
        }

        RegisterInPython(const char * name)
        {
            pyRegFunctions().push_back(boost::bind(reg_1, name));
        }

        static void reg_0()
        {
            py_register<T>();
        }

        static void reg_1(const char * name)
        {
            py_register<T>(name);
        }
    };

    inline void registerClassesInPython()
    {
        BOOST_FOREACH(PyRegFunctions::reference f, pyRegFunctions())
        {
            f();
        }
    }
}

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define MAKE_UNIQUE(x) CONCATENATE(x, __COUNTER__)

#define MARKETSIM_PY_REGISTER(T) \
    MARKETSIM_HAS_REFERENCE_TO_PYTHON(T)    \
    marketsim::RegisterInPython<T> MAKE_UNIQUE(_py_reg_);


#define MARKETSIM_PY_REGISTER_NAME(T, name) \
    MARKETSIM_HAS_REFERENCE_TO_PYTHON(T)    \
    marketsim::RegisterInPython<T> MAKE_UNIQUE(_py_reg_)(name);

#endif