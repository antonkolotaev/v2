#ifndef _marketsim_python_order_queue_h_included
#define _marketsim_python_order_queue_h_included


#include <boost/python.hpp>
#include <marketsim/order_queue/order_queue.h>
#include <marketsim/python/basic/orders.h>

namespace marketsim {
namespace basic {

        using namespace boost::python;

        typedef boost::python::object   py_object;
        
        namespace order_queue
        {
            using namespace marketsim::order_queue;
            template <Side SIDE>
            struct with_callback  :   
                OnQueueTopChanged   < py_callback,
                OrderQueue          < boost::intrusive_ptr<order::LimitT<SIDE> >, 
                with_callback       < SIDE
                > > > 
            {
                with_callback() {}

                static std::string py_name() 
                {
                    return "OrderQueue_" + marketsim::py_name<side_tag<SIDE> >();
                }

                static void py_register(std::string const & name = py_name())
                {
                    class_<with_callback, boost::noncopyable> c(name.c_str());

                    base::py_visit(c);
                }
            private:
                with_callback(with_callback const &);
            };
        }
}}
#endif