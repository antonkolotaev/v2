#ifndef _marketsim_fast_order_queue_h_included
#define _marketsim_fast_order_queue_h_included

#include "with_history_in_deque.h"
#include "limit_order.h"

namespace marketsim {
namespace fast {

    namespace order_queue 
    {
        using namespace marketsim::order_queue;
        using namespace boost::python;

        template <Side SIDE>
        struct with_history:   
            WithHistoryInDeque  <
            OrderQueue          <   boost::intrusive_ptr<order::LimitT<SIDE> >,
            with_history        < SIDE
            > > >
        {
            with_history() {}

            static std::string py_name() 
            {
                return "OrderQueue_" + marketsim::py_name<side_tag<SIDE> >();
            }

            static void py_register(std::string const & name = py_name())
            {
                class_<with_history, boost::noncopyable> c(name.c_str());

                base::py_visit(c);
            }
        private:
            with_history(with_history const &);
        };
    }


}}
#endif