#ifndef INTRUSIVE_PTR_PYTHON_HELPER_HPP
#define INTRUSIVE_PTR_PYTHON_HELPER_HPP

# include <boost/python.hpp>
# include <boost/intrusive_ptr.hpp>

// NB!!! This code is taken from http://mail.python.org/pipermail/cplusplus-sig/2007-July/012266.html


// This code helps to solve the object identity problem for intrusive pointer 
// objects that hold an embedded reference a corresponding python object.
// Note that the object must have a __del__ function that resets the 
// intrusive_ptr when the python object is deleted to avoid a dangling pointer.
// This code was copied from include/boost/python/object/make_instance.hpp 
// with some minor changes:  
//
//   (1) The template was specialized for a T object held by an
//       intrusive_ptr<T> smart pointer.  
//   (2) A test was added at the beginning of the execute member function to
//       see if a python object is already associated with the given T
//       object.  If there is an existing association, the function returns
//       a reference to the python object.  
//   (3) Otherwise, the python object is created as usual, and a pointer to
//       the python object is saved in the T object.

namespace boost { namespace python { namespace objects { 

    template <class T>
    struct make_instance_impl<
        T,
        pointer_holder<intrusive_ptr<T>, T>, 
        make_ptr_instance<T, pointer_holder<intrusive_ptr<T>, T> > >
    {
        typedef pointer_holder<intrusive_ptr<T>,T> Holder;
        typedef make_ptr_instance<T,pointer_holder<intrusive_ptr<T>,T> > Derived;

        typedef objects::instance<Holder> instance_t;

        template <class Arg>
        static inline PyObject* execute(Arg& x)
        {

            // If a python wrapper already exists, return a reference to it
            if (x && intrusive_ptr_get_pyobject(x.get()))
            {
                return incref(intrusive_ptr_get_pyobject(x.get()));
            }

            BOOST_STATIC_ASSERT(is_class<T>::value);

            PyTypeObject* type = Derived::get_class_object(x);

            if (type == 0)
                return python::detail::none();

            PyObject* raw_result = type->tp_alloc(
                type, objects::additional_instance_size<Holder>::value);

            if (raw_result != 0)
            {
                python::detail::decref_guard protect(raw_result);

                instance_t* instance = (instance_t*)raw_result;

                // construct the new C++ object and install the pointer
                // in the Python object.
                Derived::construct(&instance->storage, (PyObject*)instance, x)->install(raw_result);

                // Note the position of the internally-stored Holder,
                // for the sake of destruction
                instance->ob_size = offsetof(instance_t, storage);

                // Save a pointer to the pyobject for future reference
                intrusive_ptr_set_pyobject(x.get(), raw_result);

                // Release ownership of the python object
                protect.cancel();
            }
            return raw_result;
        }
    };

}}} // namespace boost::python::object


// This code enables the conversion of a Python None object to a 'null' 
// intrusive pointer.  This code was copied from
// include/boost/python/converter/shared_ptr_from_python.hpp with some minor 
// changes:  
//
//   (1) shared_ptr was replaced by intrusive_ptr.
//   (2) The shared_ptr_deleter argument was left off the pointer constructor.

namespace boost { namespace python { namespace converter { 

    template <class T>
    struct intrusive_ptr_from_python
    {
        intrusive_ptr_from_python()
        {
            converter::registry::insert(&convertible, &construct, type_id<intrusive_ptr<T> >());
        }

    private:
        static void* convertible(PyObject* p)
        {
            if (p == Py_None)
                return p;

            return converter::get_lvalue_from_python(p, registered<T>::converters);
        }

        static void construct(PyObject* source, rvalue_from_python_stage1_data* data)
        {
            void* const storage = ((converter::rvalue_from_python_storage<intrusive_ptr<T> >*)data)->storage.bytes;
            // Deal with the "None" case.
            if (data->convertible == source)
                new (storage) intrusive_ptr<T>();
            else
                new (storage) intrusive_ptr<T>(static_cast<T*>(data->convertible));

            data->convertible = storage;
        }
    };

}}} // namespace boost::python::converter


#endif // INTRUSIVE_PTR_PYTHON_HELPER_HPP
