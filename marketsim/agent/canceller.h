#ifndef _marketsim_agent_canceller_h_included_
#define _marketsim_agent_canceller_h_included_

#include <vector>
#include <marketsim/scheduler.h>
#include <marketsim/rng.h>
#include <boost/foreach.hpp>

namespace marketsim {
namespace agent     {

    /// This base class for agent keeps a vector of orders issued
    /// This vector can be used, for example, to cancel arbitrary orders
    /// Orders must provide functions (set/get)CancelPosition in order to speed up look up (see order::WithCancelPosition)
    /// \param Order type for storing a pointer to order (usually boost::intrusive_ptr<Order>)
    template <typename OrderPtr, typename Base>
        struct OrdersSubmittedInVector : Base 
    {
        template <typename T> OrdersSubmittedInVector(T const & x) : Base(x) {}

        /// remembers an order just created
        void processOrder(OrderPtr order)
        {
            assert(!order->cancelled());
            // store its position in our vector
            order->setCancelPosition(orders_issued_.size());
            // store the order in the vector
            orders_issued_.push_back(order);
            // continue processing
            Base::processOrder(order);
        }

        /// removes order from the vector
        void onOrderFilled(OrderPtr order)
        {
            removeOrder(order);
            Base::onOrderFilled(order);
        }

        /// removes order from the vector
        void onOrderCancelled(OrderPtr order)
        {
            removeOrder(order);
            Base::onOrderCancelled(order);
        }

    protected:

        /// Orders issued count
        size_t ordersIssuedCount() const 
        {
            return orders_issued_.size();
        }

        /// Getting access to an order with 'idx' position 
        OrderPtr& getIssuedOrder(size_t idx) 
        {
            return orders_issued_[idx];
        }

        /// \return (Price,Volume) for the idx-th order
        PriceVolume getIssuedOrderPV(size_t idx) const 
        {
            return orders_issued_[idx]->getPV();
        }

        /// removes order from the vector
        /// it must be in the vector
        void removeOrder(OrderPtr order)
        {
            size_t pos = order->getCancelPosition();

            assert(orders_issued_[pos] == order);
            erase(pos);
        }

        /// erases order with 'idx' position in the vector
        void erase(int idx) 
        {
            assert((size_t)idx < orders_issued_.size());

            // if there are at least two orders
            if (orders_issued_.size() > 1)
            {
                // copy the last one to 'idx' position
                orders_issued_[idx] = orders_issued_.back();
                // ... and update its cancel position
                orders_issued_[idx]->setCancelPosition(idx);
            }

            // remove the last order that's not needed anymore
            orders_issued_.pop_back();
        }

        /// sends a cancellation signal to idx-th order
        void cancelOrder(size_t idx)
        {
            assert(idx < orders_issued_.size());
            orders_issued_[idx]->onCancelled();
        }

#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T>
            static void py_visit(T & c)
            {
                Base::py_visit(c);
                c.def("ordersIssuedCount", &OrdersSubmittedInVector::ordersIssuedCount);
                c.def("getIssuedOrderPV", &OrdersSubmittedInVector::getIssuedOrderPV);
                c.def("cancelOrder", &OrdersSubmittedInVector::cancelOrder);
            }
#endif

        /// Checking that all orders in the vector are not cancelled
        void check_all_orders_in_queue()
        {
            BOOST_FOREACH(OrderPtr order, orders_issued_)
            {
                typename order_side<Order>::type  tag;
                assert(Base::getOrderBook()->orderQueue(tag).contains(order));
            }
        }
    private:
        std::vector<OrderPtr>     orders_issued_;
    };

    /// Base class for agents implementing random order cancellation
    /// In moments of time given by CancelInterval distribution 
    /// it chooses an integer i using IndexChooser distribution
    /// and sends a cancellation signal for i-th order
    template <typename CancelInterval, typename OrderPtr, typename Base, typename IndexChooser = rng::uniform_smallint<> >
        struct OrderCanceller : OrdersSubmittedInVector<OrderPtr, Base>
    {
        typedef Timer<OrderCanceller, CancelInterval>       timer_t;
        typedef OrdersSubmittedInVector<OrderPtr, Base>     RealBase;

        /// 0-th argument is passed to the base class
        /// 1-th argument defines interval distribution between order cancellations
        template <typename T>  OrderCanceller(T const & x) 
            : RealBase (boost::get<0>(x))
            , timer_   (*this->self(), &OrderCanceller::cancelAnOrder, boost::get<1>(x))
        {
        }

        DECLARE_BASE(OrderCanceller);
        
    protected:

        /// cancels a randomly chosen order
        void cancelAnOrder()
        {
            if (size_t n = RealBase::ordersIssuedCount())
            {
                int idx = IndexChooser(0, n - 1)();

                RealBase::getIssuedOrder(idx)->onCancelled();
            }
        }

    private:
        timer_t                timer_;
    };


}}

#endif
