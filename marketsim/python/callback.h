#ifndef _marketsim_python_basic_callback_h_included
#define _marketsim_python_basic_callback_h_included

#include <boost/python.hpp>
#include <boost/foreach.hpp>
#include <list>
#include <algorithm>
#include <marketsim/common_types.h>

namespace marketsim {

    using namespace boost::python;

    typedef boost::python::object   py_object;

    template <class T>
    struct py_value
    {
        py_value(py_object value) : value(value) {}

        T operator () ()
        {
            return boost::python::extract<T>(value());
        }

        py_object value;
    };

    struct py_callback
    {   
        py_callback(py_object c) 
        {
            callbacks.push_back(c);
        }

        py_callback(Dummy) {}       
        py_callback() {}

        template <class T>
            void operator () (T * x)
            {
                reference_existing_object::apply<T*>::type converter;
                PyObject* obj = converter( x );
                object real_obj = object( handle<>( obj ) );

                BOOST_FOREACH(py_object callback, callbacks)
                {
                    if (!callback.is_none())
                    {
                        callback(real_obj);
                    }
                }
            }

        void add(py_object x)
        {
            callbacks.push_back(x);
        }

        void remove(py_object x)
        {
            callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), x), callbacks.end());
        }


        static void py_register()
        {
            class_<py_callback>("py_callback")
                .def("add", &py_callback::add)
                .def("remove", &py_callback::remove)
                ;
        }

        std::list<py_object>   callbacks;
    };
}

#endif