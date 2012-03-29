#ifndef _marketsim_agent_for_order_h_included_
#define _marketsim_agent_for_order_h_included_

#include <marketsim/common/PriceVolume.h>

namespace marketsim {
namespace agent {

    template <typename Order>
        struct IAgentForMarketOrder 
    {
        /// called when volume in order diminished after a trade
        virtual void onOrderPartiallyFilled(Order *, PriceVolume const &) = 0;
        /// called when an order is completely filled
        virtual void onOrderFilled   (Order *) = 0;
    };


    /// When there are limit order issuers of different types in a simulation
    /// we have to parametrize orders by some class which is base for all liquidity providers in hand
    /// This class is interface used by orders to notify agents about events occurred (see order/link_to_agent.h)
    template <typename Order>
        struct IAgentForOrder : IAgentForMarketOrder<Order>
    {
        /// called when an order is cancelled
        virtual void onOrderCancelled(Order *) = 0;
    };

    /// Default implementation for IAgentForOrder
    /// Simply passes calls to the base class
    /// Motivation for this class -- to get rid of concrete order type for other agent base classes
    template <typename Order, typename Base>
        struct IAgentForOrderImpl : Base, IAgentForOrder<Order>
    {
        template <class T>
        IAgentForOrderImpl(T const & x)
            :   Base(x)
        {}

        DECLARE_BASE(IAgentForOrderImpl);

        using Base::onOrderPartiallyFilled;
        using Base::onOrderFilled;
        using Base::onOrderCancelled;

        void onOrderPartiallyFilled(Order * order, PriceVolume const & x) 
        {
            Base::onOrderPartiallyFilled(order, x);
        }

        void onOrderFilled(Order *order)
        {
            Base::onOrderFilled(order);
        }

        void onOrderCancelled(Order * order)
        {
            Base::onOrderCancelled(order);
        }
    };

   template <typename OrderA, typename OrderB, typename Base>
       struct IAgentForMarketOrderImpl 
           : Base
           , IAgentForMarketOrder<OrderA>
           , IAgentForMarketOrder<OrderB>   
       {
           template <class T>
           IAgentForMarketOrderImpl(T const & x)
               :   Base(x)
           {}

           DECLARE_BASE(IAgentForMarketOrderImpl);

           using Base::onOrderPartiallyFilled;
           using Base::onOrderFilled;
           using Base::onOrderCancelled;

           void onOrderPartiallyFilled(OrderA * order, PriceVolume const & x) 
           {
               Base::onOrderPartiallyFilled(order, x);
           }

           void onOrderPartiallyFilled(OrderB * order, PriceVolume const & x) 
           {
               Base::onOrderPartiallyFilled(order, x);
           }

           void onOrderFilled(OrderA *order)
           {
               Base::onOrderFilled(order);
           }

           void onOrderFilled(OrderB *order)
           {
               Base::onOrderFilled(order);
           }
       };

}}

#endif