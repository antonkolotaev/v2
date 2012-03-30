#ifndef _marketsim_agent_order_pool_h_included_
#define _marketsim_agent_order_pool_h_included_

#include <marketsim/object_pool.h>
#include <boost/iterator/iterator_traits.hpp>

namespace boost 
{
    template <class T>
    struct iterator_value<intrusive_ptr<T> >
    {
        typedef T type;
    };
}

namespace marketsim {
namespace agent
{
    /// Base class for agents that use a pool to create orders in
    template <
        typename OrderPtr,     // type of orders to create
        typename Base
    >
        struct SharedOrderPool : Base 
    {
        SharedOrderPool() {}

        template <typename T>
            SharedOrderPool(T const & x) 
                : Base(x) 
            {}

        DECLARE_BASE(SharedOrderPool);

        typedef typename boost::iterator_value<OrderPtr>::type  Order;
        typedef Order order_type;

        using Base::derived_t;

        /// creates an order in the pool
        /// \param T x - parameters for order constructor
        /// \return a raw pointer to the order created (should it be a smart pointer?)
        template <typename T>
            OrderPtr createOrder(T const & x)
            {
                return new (pool_.alloc()) Order(x, &pool_, static_cast<derived_t*>(this));
            }

   private:
       static object_pool<Order>   pool_;
   };

    template <
        typename OrderPtr,     // type of orders to create
        typename Base
    >
    object_pool<typename boost::iterator_value<OrderPtr>::type>  SharedOrderPool<OrderPtr, Base>::pool_;

}}

#endif
