#ifndef _marketsim_order_base_h_included_
#define _marketsim_order_base_h_included_


#include <vector>
#include <utility>
#include <marketsim/common_types.h>
#include <marketsim/order/volume_holder.h>
#include <marketsim/order/side.h>
#include <marketsim/order/price_holder.h>

#include <marketsim/order/limit_order_base.h>
#include <marketsim/order/market_order_base.h>

namespace marketsim
{

    template <typename T>  
        std::ostream & operator << (std::ostream &out, boost::intrusive_ptr<T> const & p)
        {
            return out << *p;
        }

}

#endif