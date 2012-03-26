#ifndef _marketsim_fast_order_book_h_included
#define _marketsim_fast_order_book_h_included

#include "order_queue.h"
#include <marketsim/order_book.h>

namespace marketsim {
namespace fast {

    struct OrderBook : 
        marketsim::OrderBook     < order_queue::with_history<Buy>, order_queue::with_history<Sell>,
        marketsim::PyRefCounted  < IRefCounted >
        >    
    {
        OrderBook() {}

        static void py_register()
        {
            using namespace boost::python;

            class_<OrderBook, boost::intrusive_ptr<OrderBook>, bases<IRefCounted>, boost::noncopyable> c("OrderBook", no_init);

            base::py_visit(c);

            register_0<OrderBook>(c);
        }

    private:
        OrderBook(OrderBook const &);
    };

}}

MARKETSIM_PY_REGISTER(marketsim::fast::OrderBook)

#endif