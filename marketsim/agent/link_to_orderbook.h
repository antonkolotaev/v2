#ifndef _marketsim_agent_link_to_orderbook_h_included_
#define _marketsim_agent_link_to_orderbook_h_included_

namespace marketsim {
namespace agent {

    /// This base class for agents holds a link to order book where the agent sends orders
    template <typename BookPtr, typename Base>
        struct LinkToOrderBook : Base
    {
        /// 0-th argument is passed to the base class
        /// 1-th argument holds a pointer to the order book
        template <typename T>
            LinkToOrderBook(T const & x) 
            :   Base       (boost::get<0>(x))
            ,   order_book_(boost::get<1>(x))
        {}

        /// \returns pointer to the order book
        BookPtr getOrderBook() { return order_book_; }

        /// transfers an order to the order book
        template <typename T>
            void processOrder(T x)
            {
                order_book_->processOrder(x);
            }

        /// notifies the order book about an order cancellation
        template <typename T>
            void onOrderCancelled(T x) 
        {
            order_book_->onOrderCancelled(x);
        }

#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T>
            static void py_visit(T & c)
            {
                using namespace boost::python;
                Base::py_visit(c);
                c.def("orderBook", &LinkToOrderBook::getOrderBook/*, return_internal_reference<>()*/);
            }
#endif

    private:
        BookPtr    order_book_;
    };

}}

#endif