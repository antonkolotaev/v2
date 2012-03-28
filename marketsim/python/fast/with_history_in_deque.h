#ifndef _marketsim_python_fast_with_history_in_deque_h_included_
#define _marketsim_python_fast_with_history_in_deque_h_included_

#include <marketsim/order_queue/order_queue.h>
#include <marketsim/order_queue/best_price_and_volume.h>
#include <marketsim/history.h>
#include <marketsim/order_queue/on_queue_top_changed.h>

namespace marketsim {
namespace order_queue {

   template <typename Base>
        struct WithHistoryInDeque : 
            OnQueueTopChanged<history::Collector<BestPriceAndVolume, history::InDeque<PriceVolume> >, Base>
        {
            typedef history::Collector<BestPriceAndVolume, history::InDeque<PriceVolume> > TAG;

            typedef OnQueueTopChanged<TAG, Base> RealBase;

            history::TimeSeries<PriceVolume> const & getHistory() const 
            {
                TAG *tag = 0;
                const_cast<WithHistoryInDeque*>(this)->getHandler(tag).flush();
                return  getHandler(tag).getHistory(); 
            }

            void recordHistory(bool b)
            {
                getHandler((TAG*)0).recordHistory(b);
            }

            DECLARE_BASE(WithHistoryInDeque);

#ifdef MARKETSIM_BOOST_PYTHON
            template <typename T>
                static void py_visit(T & class_def)
                {
                    using namespace boost::python;
                    py_register<history::TimeSeries<PriceVolume> >();
                    RealBase::py_visit(class_def);        
                    class_def
                        .def("history", &WithHistoryInDeque::getHistory, return_internal_reference<>())
                        .def("recordHistory", &WithHistoryInDeque::recordHistory)
                        ;
                }
#endif
        };

}}

#endif