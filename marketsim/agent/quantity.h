#ifndef _marketsim_agent_quantity_h_included_
#define _marketsim_agent_quantity_h_included_

#include <queue>

#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim
{
    template <typename Base>       
        struct Quantity_Holder : Base
    {
        template <typename T>
            Quantity_Holder(T const & x) : Base(x), quantity_(0) {}

        typedef Quantity_Holder    base; // for derived typenamees

        void setQuantity(Volume v) { quantity_ = v; }
        Volume getQuantity() const { return quantity_; }

        template <typename Order>
            void onOrderPartiallyFilled(Order order, PriceVolume const & x)
        {
            quantity_ += order->calculateQuantity(x.volume);
            Base::onOrderPartiallyFilled(order, x);
        }

    private:
        Volume  quantity_;
    };

    struct Quantity 
    {
        typedef Volume  ValueType;

        template <typename T>
            static ValueType getValue(T x)
            {
                return x->getQuantity();
            }

        template <typename Base>
            struct base 
        {
            typedef Quantity_Holder<Base>   type;
        };
    };
    

    typedef std::pair<Time, Volume>  QuantityHistoryPiece;
}

#endif