#ifndef _marketsim_order_in_pool_h_included_
#define _marketsim_order_in_pool_h_included_

#include <boost/tuple/tuple.hpp>
#include <marketsim/common_types.h>
#include <marketsim/ref_counted.h>

namespace marketsim {
namespace order     {
    // The idea of this class is to provide non-virtual function on_released 
    // that calls 'free' method of a class indirectly defined by HolderFunc

    /// In order to avoid excessive and frequent memory allocations for orders
    /// we may use a dedicated memory allocator (see object_pool<T> for example)
    /// When a reference counted order is to be freed (its reference count become 0),
    /// we ask the memory allocator to note that the memory of the order may be re-used
    
    /// Base class for orders allocated in a pool
    /// \param HolderPtrF meta class (i.e. a class having 'apply' meta function) 
    ///                   that returns type of for a pointer to the pool 
    ///                   (we cannot use the type directly in order to break cyclic type dependencies)
    /// NB! This class derives from RefCounted -> better design should be proposed to avoid clashing for example with RefCounter in a timer
    /// TODO: derive optionally order base classes from RefCounted
    template <typename HolderPtrF, typename Base> 
        struct InPool : Base, RefCounted<typename Base::derived_t>
        {
            /// 0-th argument is an initializer for the base class
            /// 1-th argument is a pointer to the pool
            template <typename T>
                InPool(T const & x) 
                    : Base      (boost::get<0>(x))
                    , holder_   (boost::get<1>(x)) 
                {}

            DECLARE_BASE(InPool);

            typedef typename HolderPtrF::apply<base::derived_t>::type  holder_t;

            /// \return true iff the order is allocated in the given pool
            bool is_in_my_pool(holder_t h) const 
            {
                return h == holder_;
            }

            ~InPool() 
            {
                //std::cout << "~LimitOrder(" << price << ", " << volume << ")" << std::endl;
            }
        protected:
            /// called from RefCounted when reference count become 0
            void on_released()
            {
                // notify the pool that the memory used by the order may be freed
                holder_->free(self());
            }

            friend struct RefCounted<typename Base::derived_t>;

        private:
            holder_t  holder_;
        };
}}

#endif