#ifndef _marketsim_agent_canceller_h_included_
#define _marketsim_agent_canceller_h_included_

#include <vector>
#include <marketsim/scheduler.h>
#include <marketsim/rng.h>

namespace marketsim
{
    template <typename CancelInterval, typename Order, typename Base, class IndexChooser = rng::uniform_smallint >
        struct OrderCanceller : Base
    {
        typedef Timer<OrderCanceller, CancelInterval>   timer_t;

        template <class T>  OrderCanceller(T const & x) 
            : Base     (boost::get<0>(x))
            , timer_   (*self(), &OrderCanceller::cancelAnOrder, boost::get<1>(x))
        {
        }

        const static int cancelledIdx = -0xbad;

        typedef OrderCanceller base;    // for derived typenamees
        
        template <typename Order>
            void processOrder(Order order)
            {
                order->setCancelPosition(orders_issued_.size());
                orders_issued_.push_back(order);
                Base::processOrder(order);
            }

        template <typename Order>
            void onOrderFilled(Order order)
            {
                removeOrder(order);
                Base::onOrderFilled(order);
            }

        template <typename Order>
            void onOrderCancelled(Order order)
            {
                removeOrder(order);
                Base::onOrderCancelled(order);
            }

    private:

        template <typename Order>
            void removeOrder(Order order)
        {
            size_t pos = order->getCancelPosition();

            if (pos != cancelledIdx)
            {
                assert(orders_issued_[pos] == order);
                erase(pos);
            }
        }

        void cancelAnOrder()
        {
            if (!orders_issued_.empty())
            {
                int pos = IndexChooser(0, orders_issued_.size() - 1)();

                Order order = orders_issued_[pos];
                erase(pos);

                order->setCancelPosition(cancelledIdx);
                order->onCancelled();
            }
        }

        void erase(int pos) 
        {
            assert((size_t)pos < orders_issued_.size());

            if (orders_issued_.size() > 1)
            {
                orders_issued_[pos] = orders_issued_.back();
                orders_issued_[pos]->setCancelPosition(pos);
            }

            orders_issued_.pop_back();
        }

    private:
        std::vector<Order>     orders_issued_;
        timer_t                timer_;
    };


}

#endif