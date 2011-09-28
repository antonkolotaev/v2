#ifndef _marketsim_agent_order_pool_h_included_
#define _marketsim_agent_order_pool_h_included_

#include <marketsim/object_pool.h>

namespace marketsim
{
   template <typename Order, typename Base>
        struct PrivateOrderPool : Base 
   {
        template <typename T>
            PrivateOrderPool(T const & x) : Base(x) {}

        typedef PrivateOrderPool base; // for derived typenamees

        typedef Order  order_type;
        typedef Order* order_ptr_t;

        template <typename T>
            order_ptr_t createOrder(T const & x)
            {
                return new (pool_.alloc()) Order(x, &pool_, self());
            }

   private:
       object_pool<Order>   pool_;
   };
}

#endif
