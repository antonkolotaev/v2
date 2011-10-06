#ifndef _marketsim_agent_PnL_h_included_
#define _marketsim_agent_PnL_h_included_

#include <marketsim/common_types.h>

namespace marketsim
{
    template <typename Base>
        struct PnL_Holder : Base 
        {
            typedef PnL_Holder     base;   // for derived typenamees

            template <typename T>
                PnL_Holder(T const x) 
                    :   Base(x)
                    ,   PnL_(0)
                {}

            template <typename Order>
                void onOrderPartiallyFilled(Order order, PriceVolume const & x)
            {
                PnL_ += order->calculatePnL(x);
                Base::onOrderPartiallyFilled(order, x);
            }

            Price getPnL() const { return PnL_; }

#ifdef MARKETSIM_BOOST_PYTHON
            template <typename T>
                static void py_visit(T & c)
                {
                    Base::py_visit(c);
                    c.def_readonly("PnL", &PnL_Holder::PnL_);
                }
#endif

        private:
            Price   PnL_;
        };

    //typedef std::pair<Time, Price>  PnLHistoryPiece;

    struct PnL 
    {
        typedef Price  ValueType;

        template <typename T>
            static ValueType getValue(T x)
            {
                return x->getPnL();
            }

        template <typename Base>
            struct base 
        {
            typedef PnL_Holder<Base>   type;
        };
    };

}

#endif