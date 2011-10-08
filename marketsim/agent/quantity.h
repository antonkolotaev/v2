#ifndef _marketsim_agent_quantity_h_included_
#define _marketsim_agent_quantity_h_included_

#include <queue>

#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim {
namespace agent 
{
    /// Base class for agents tracking available amount of agent's assets
    template <typename Base>       
        struct Quantity_Holder : Base
    {
        template <typename T>
            Quantity_Holder(T const & x) 
                :   Base     (x)
                ,   quantity_(0) 
            {}

        DECLARE_BASE(Quantity_Holder);

        /// changes current assets quantity
        void setQuantity(Volume v) { quantity_ = v; }
        /// \returns current assets quantity
        Volume getQuantity() const { return quantity_; }

        /// amount of assets available is updated when an order issued by the agent is partially filled
        template <typename Order>
            void onOrderPartiallyFilled(Order order, PriceVolume const & x)
        {
            quantity_ += order->calculateQuantity(x.volume);
            Base::onOrderPartiallyFilled(order, x);
        }
#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T>
            static void py_visit(T & c)
            {
                Base::py_visit(c);
                c.def_readwrite("assetsAvailable", &Quantity_Holder::quantity_);
            }
#endif

    private:
        Volume  quantity_;
    };

    /// A traits class use to tell to statistics classes how to extract amount of assets available from an agent
    struct Quantity 
    {
        typedef Volume  ValueType;

        template <typename T>
            static ValueType getValue(T x)
            {
                return x->getQuantity();
            }
    };
    

}}

#endif