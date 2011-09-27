#ifndef _marketsim_order_in_pool_h_included_
#define _marketsim_order_in_pool_h_included_

#include <boost/tuple/tuple.hpp>
#include <marketsim/common_types.h>
#include <marketsim/ref_counted.h>

namespace marketsim
{
    // The idea of this class is to provide non-virtual function on_released 
    // that calls 'free' method of a class indirectly defined by HolderFunc
    template <typename Holder, typename Base> 
        struct InPool : Base, RefCounted<typename Base::derived_t>
        {
            template <typename T>
                InPool(T const & x) 
                    : Base(boost::get<0>(x))
                    , holder_(boost::get<1>(x)) 
                {}

            typedef InPool base; // for derived typename

            void on_released()
            {
                holder_->free(self());
            }

            ~InPool() 
            {
                //std::cout << "~LimitOrder(" << price << ", " << volume << ")" << std::endl;
            }

        private:
            typename Holder::apply<base::derived_t>::type  holder_;
        };

/*
    template <typename Base>
        struct HasRefCounter : Base, RefCounted<typename Base::derived_t>
    {
        template <typename T>
            HasRefCounter(T const & x) : Base(x) {}

        typedef HasRefCounter base;     // for derived class
    };
*/

}

#endif