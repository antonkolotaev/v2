#ifndef _marketsim_agent_link_to_orderbook_h_included_
#define _marketsim_agent_link_to_orderbook_h_included_

namespace marketsim
{
    template <class BookPtr, class Base>
        struct LinkToOrderBook : Base
    {
        template <class T>
            LinkToOrderBook(T const & x) 
            :   Base(x)
            ,   order_book_(0)
        {}

        BookPtr getOrderBook() { return order_book_; }
        void setOrderBook(BookPtr b) { order_book_ = b; }

        template <class T>
            void processOrder(T x)
            {
                order_book_->processOrder(x);
            }

        template <class T>
            void onOrderCancelled(T x) 
        {
            order_book_->onOrderCancelled(x);
        }


    private:
        BookPtr    order_book_;
    };

}

#endif