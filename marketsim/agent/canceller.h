#ifndef _marketsim_agent_canceller_h_included_
#define _marketsim_agent_canceller_h_included_

#include <vector>
#include <marketsim/scheduler.h>
#include <marketsim/rng.h>
#include <boost/foreach.hpp>

namespace marketsim
{
    template <typename Order, typename Base>
        struct OrdersSubmittedInVector : Base 
    {
        template <typename T> OrdersSubmittedInVector(T const & x) : Base(x) {}

        const static int cancelledIdx = -0xbad;

        template <typename Order>
            void processOrder(Order order)
        {
            assert(!order->cancelled());
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

    protected:

        size_t ordersIssuedCount() const 
        {
            return orders_issued_.size();
        }

        Order& getIssuedOrder(size_t idx) 
        {
            return orders_issued_[idx];
        }

        PriceVolume getIssuedOrderPV(size_t idx) const 
        {
            return orders_issued_[idx]->getPV();
        }

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

        void cancelOrder(size_t idx)
        {
            assert(idx < orders_issued_.size());
            orders_issued_[idx]->onCancelled();
        }

        template <typename T>
            static void py_visit(T & c)
            {
                Base::py_visit(c);
                c.def("ordersIssuedCount", &OrdersSubmittedInVector::ordersIssuedCount);
                c.def("getIssuedOrderPV", &OrdersSubmittedInVector::getIssuedOrderPV);
                c.def("cancelOrder", &OrdersSubmittedInVector::cancelOrder);
            }

        void check_all_orders_in_queue()
        {
            BOOST_FOREACH(Order order, orders_issued_)
            {
                typename order_side<Order>::type  tag;
                assert(Base::getOrderBook()->orderQueue(tag).contains(order));
            }
        }
    private:
        std::vector<Order>     orders_issued_;
    };

    template <typename CancelInterval, typename Order, typename Base, typename IndexChooser = rng::uniform_smallint<> >
        struct OrderCanceller : OrdersSubmittedInVector<Order, Base>
    {
        typedef Timer<OrderCanceller, CancelInterval>   timer_t;
        typedef OrdersSubmittedInVector<Order, Base>    RealBase;

        template <typename T>  OrderCanceller(T const & x) 
            : RealBase (boost::get<0>(x))
            , timer_   (*self(), &OrderCanceller::cancelAnOrder, boost::get<1>(x))
        {
        }

        DECLARE_BASE(OrderCanceller);
        
    protected:

        void cancelAnOrder()
        {
            if (size_t n = RealBase::ordersIssuedCount())
            {
                int pos = IndexChooser(0, n - 1)();

                RealBase::getIssuedOrder(pos)->onCancelled();
            }
        }

    private:
        timer_t                timer_;
    };


}

#endif