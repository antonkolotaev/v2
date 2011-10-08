#ifndef _marketsim_agent_order_pool_h_included_
#define _marketsim_agent_order_pool_h_included_

#include <marketsim/object_pool.h>

namespace marketsim {
namespace agent
{
    /// Base class for agents that use a pool to create orders in
    /// This pool is private for each agent; we might use a shared pool thus diminished memory footprint
    template <
        typename Order,     // type of orders to create
        typename Base
    >
        struct PrivateOrderPool : Base 
    {
        template <typename T>
            PrivateOrderPool(T const & x) 
                : Base(x) 
            {}

        DECLARE_BASE(PrivateOrderPool);

        typedef Order  order_type;
        typedef Order* order_ptr_t;

        /// creates an order in the pool
        /// \param T x - parameters for order constructor
        /// \return a raw pointer to the order created (should it be a smart pointer?)
        template <typename T>
            order_ptr_t createOrder(T const & x)
            {
                return new (pool_.alloc()) Order(x, &pool_, self());
            }

        /// Since all orders a stored in an order queue, we ask the order queue to remove all our orders
        /// it is reasonable since without agent (which is about to be destroyed) its orders have no sense
        ~PrivateOrderPool()
        {
            self()->getOrderBook()->orderQueue(typename order_side<Order>::type()).remove_all_pool_orders(&pool_);
        }

   private:
       object_pool<Order>   pool_;
   };
}}

#endif
