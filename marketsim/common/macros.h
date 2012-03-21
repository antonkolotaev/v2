#ifndef _marketsim_common_macros_h_included_
#define _marketsim_common_macros_h_included_

/*
    We shall build concrete classes from parts by inheriting them in chain:
    class X :
        A < Params_for_A, 
        B < Params_for_B, 
        C < Params_for_C,
        .... 
        X_Base <X> > > > 

    Normally the most base class is parametrized by the most derived one in order to provide access 
    from base classes to the derived ones by static downcasting of this pointer.

    We follow the agreement that the most derived class can be accessed through self() function 
    that should be declared in the most base class by using, for example, macros DECLARE_ARROW(most_derived_class_name)

    Some classes can be parametrized by 'value' types and pointer-like types
    In order to unify these two cases overloads to operator-> and operator* are to be introduced to such types
    It allows to consider all parameters as pointers: param->method()

    The only exceptions so far are functor-like parameters exposing operator () 
    In order to be compatible with third party libraries (like Boost.Random or Boost.Python) 
    we consider them as passed by value: param()

    Since concrete classes usually derive from a class with very long name (like A<params_for_A, B < params_for_B, ... )
    In order to abbreviate call to the constructor of the base class we introduce 
    typename base in all template classes suspected to be base ones
    To do this declaration we use macros DECLARE_BASE(this_class_name)
*/

/// Introduces a pointer-like semantics to class and defines self() method providing access to the most derived class
/// X is the most derived class
/// This macros should be used in the most base class in an hierarchy
#define DECLARE_ARROW(X) \
    X *         self ()      { return  static_cast<X*>(this); } \
    X const *   self () const{ return  static_cast<X const *>(this); } \
    X *         operator -> ()      { return  static_cast<X*>(this); } \
    X const *   operator -> () const{ return  static_cast<X const *>(this); } \
    X &         operator * ()       { return  static_cast<X&>(*this); } \
    X const &   operator * () const { return  static_cast<X const&>(*this); } 

/// Declares typename base == X. 
/// X is supposed to be equal to current class
/// This macro eases access to base class from derived concrete classes
#define DECLARE_BASE(X) typedef X base; 

namespace marketsim
{
    template <class Derived>
        struct derived_is
        {
            typedef Derived derived_t;
            DECLARE_ARROW(Derived);
        };
}

#endif