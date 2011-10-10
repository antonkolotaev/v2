#ifndef _marketsim_collect_in_deque_h_included_
#define _marketsim_collect_in_deque_h_included_

#include <marketsim/history/collector.h>
#include <marketsim/history/in_deque.h>

namespace marketsim {
namespace history   {

    /// Combines a history collector with a storage in deque
    /// \param FieldTag - tag class telling how to extract values to be stored from the underlying type
    template <class FieldTag>   
        struct CollectInDeque : Collector<FieldTag>
        {
            typedef Collector<FieldTag>         Base;
            typedef typename Base::FieldType    FieldType;

            template <class T> 
                CollectInDeque(T const & x) : Base(x)
                {}

            CollectInDeque() {}

            /// Since history::Collector doesn't push the last value to storage
            /// we force it to do it
            TimeSeries<FieldType> const & getHistory() 
            {
                Base::flush();
                return Base::getHistory();
            }

#ifdef MARKETSIM_BOOST_PYTHON 

            static std::string py_name() 
            {
                return "HistoryInDeque_" + marketsim::py_name<FieldTag>();
            }

            static void py_register()
            {
                using namespace boost::python;
                ::py_register<HistoryStorage>();
                class_<CollectInDeque, boost::noncopyable> c(py_name().c_str());
                c.def("getHistory", &CollectInDeque::getHistory, return_internal_reference<>());
                Base::py_visit(c);
            }
#endif
        };


}}

#endif