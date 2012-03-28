#ifndef _marketsim_python_order_book_h_included
#define _marketsim_python_order_book_h_included

#include <marketsim/order_book.h>
#include <marketsim/python/basic/order_queue.h>

namespace marketsim {
namespace basic {

    using namespace boost::python;
    typedef boost::python::object   py_object;

    struct OrderBook : 
       marketsim::OrderBook < order_queue::with_callback<Buy>, order_queue::with_callback<Sell>, 
       PyRefCounted         < IRefCounted >
       >    
    {
       OrderBook() {}

       static void py_register(const char *name)
       {
           class_<
               OrderBook, 
               boost::intrusive_ptr<OrderBook>, 
               boost::noncopyable
           > 
           c(name, no_init);

           base::py_visit(c);

           marketsim::register_0<OrderBook>(c);
       }

    private:
       OrderBook(OrderBook const &);
    };
}}

#endif
