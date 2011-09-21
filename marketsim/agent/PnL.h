#ifndef _marketsim_agent_PnL_h_included_
#define _marketsim_agent_PnL_h_included_

#include <marketsim/common_types.h>

namespace marketsim
{
    template <class Base>
        struct PnL_Holder : Base 
        {
            typedef PnL_Holder     base;   // for derived classes

            template <class T>
                PnL_Holder(T const x) 
                    :   Base(x)
                    ,   PnL_(0)
                {}

            template <class Order>
                void onOrderPartiallyFilled(Order order, PriceVolume const & x)
            {
                PnL_ += order->calculatePnL(x);
                Base::onOrderPartiallyFilled(order, x);
            }

            Price getPnL() const { return PnL_; }

        private:
            Price   PnL_;
        };

    typedef std::pair<Time, Price>  PnLHistoryPiece;

    struct PnL 
    {
        typedef Price  ValueType;

        template <class T>
            static ValueType getValue(T x)
            {
                return x->getPnL();
            }

        template <class Base>
            struct base 
        {
            typedef PnL_Holder<Base>   type;
        };
    };

}

#endif