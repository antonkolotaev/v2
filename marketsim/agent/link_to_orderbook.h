#ifndef _marketsim_agent_link_to_orderbook_h_included_
#define _marketsim_agent_link_to_orderbook_h_included_

namespace marketsim
{
    template <typename BookPtr, typename Base>
        struct LinkToOrderBook : Base
    {
        template <typename T>
            LinkToOrderBook(T const & x) 
            :   Base       (boost::get<0>(x))
            ,   order_book_(boost::get<1>(x))
        {}

        BookPtr getOrderBook() { return order_book_; }

        template <typename T>
            void processOrder(T x)
            {
                order_book_->processOrder(x);
            }

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
                c.def("orderBook", &LinkToOrderBook::getOrderBook, return_internal_reference<>());
            }
#endif


    private:
        BookPtr    order_book_;
    };

}

#endif