#ifndef _marketsim_agent_PnL_h_included_
#define _marketsim_agent_PnL_h_included_

#include <marketsim/common_types.h>

namespace marketsim
{
    /// Base class for agents tracking P&L for the agent
    template <typename Base>
        struct PnL_Holder : Base 
        {
            DECLARE_BASE(PnL_Holder);

            template <typename T>
                PnL_Holder(T const x) 
                    :   Base(x)
                    ,   PnL_(0)
                {}

            /// P&L value is updated when an order issued by the agent is partially filled
            template <typename Order>
                void onOrderPartiallyFilled(Order order, PriceVolume const & x)
            {
                PnL_ += order->calculatePnL(x);
                Base::onOrderPartiallyFilled(order, x);
            }

            /// \return current P&L value
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

    /// A traits class use to tell to statistics classes how to extract P&L value from an agent
    struct PnL 
    {
        typedef Price  ValueType;

        template <typename T>
            static ValueType getValue(T x)
            {
                return x->getPnL();
            }
    };

}

#endif