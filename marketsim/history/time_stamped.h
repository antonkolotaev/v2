#ifndef _marketsim_history_time_stamped_h_included_
#define _marketsim_history_time_stamped_h_included_

#include <marketsim/scheduler.h>

namespace marketsim {
namespace history {

    /// a value with a time stamp; a pair (Time, T)
    /// \param T - the value type
    /// to be moved to history/time_stamped.h
    template <typename T>
        struct TimeStamped 
        {
            Time    time;
            T       value;

            TimeStamped(Time t, T const &x)
                :   time(t), value(x)
            {}

            friend bool operator == (TimeStamped const & lhs, TimeStamped const & rhs)
            {
                return lhs.time == rhs.time && lhs.value == rhs.value;
            }

            friend bool operator != (TimeStamped const & lhs, TimeStamped const & rhs)
            {
                return !(lhs == rhs);
            }

            template <typename Stream>
                friend Stream& operator << (Stream &out, TimeStamped const & x)
                {
                    out << "{ t=" << x.time << " " << x.value << "}";
                    return out;
                }

#ifdef MARKETSIM_BOOST_PYTHON
            static std::string py_name() { return "TimeStamped_" + T::py_name(); }

            static void py_register(std::string const & name = py_name())
            {
                using namespace boost::python;

                ::py_register<T>();

                class_<TimeStamped>(name.c_str(), init<Time, T>())
                    .def_readonly("time",  &TimeStamped::time)
                    .def_readonly("value", &TimeStamped::value)
                    .def("__str__", &toStr<TimeStamped>)
                    .def("__repr__", &toStr<TimeStamped>)
                    ;
            }
#endif
        };

}}

#endif