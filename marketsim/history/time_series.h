#ifndef _marketsim_history_time_series_h_included_
#define _marketsim_history_time_series_h_included_

#include <queue>
#include <marketsim/history/time_stamped.h>

namespace marketsim {
namespace history {

    /// Time series of values of type T using std::deque as storage
    template <typename T>
        struct TimeSeries
            :   std::deque<TimeStamped<T> >
        {
#ifdef MARKETSIM_BOOST_PYTHON
            static std::string py_name() 
            {
                return "TimeSeries_" + T::py_name();
            }

            static void py_register(std::string const &name = py_name())
            {
                using namespace boost::python;
                ::py_register<TimeStamped<T> >();

                class_<TimeSeries>(name.c_str())
                    .def("__iter__", boost::python::iterator<TimeSeries>())
                    ;
            }
#endif
        };


}}

#endif