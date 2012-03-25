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
        struct SharedOrderPool : Base 
    {
        template <typename T>
            SharedOrderPool(T const & x) 
                : Base(x) 
            {}

        DECLARE_BASE(SharedOrderPool);

        typedef Order  order_type;
        typedef Order* order_ptr_t;

        /// creates an order in the pool
        /// \param T x - parameters for order constructor
        /// \return a raw pointer to the order created (should it be a smart pointer?)
        template <typename T>
            order_ptr_t createOrder(T const & x)
            {
                return new (pool_.alloc()) Order(x, &pool_, this->self());
            }

   private:
       static object_pool<Order>   pool_;
   };

    template <
        typename Order,     // type of orders to create
        typename Base
    >
    object_pool<Order>  SharedOrderPool<Order, Base>::pool_;

}}

#endif
