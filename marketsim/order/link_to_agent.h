#ifndef _marketsim_order_link_to_agent_h_included_
#define _marketsim_order_link_to_agent_h_included_

namespace marketsim 
{
    template <typename Order>
        struct IAgentForOrder 
    {
        virtual void onOrderPartiallyFilled(Order *, PriceVolume const &) = 0;
        virtual void onOrderFilled   (Order *) = 0;
        virtual void onOrderCancelled(Order *) = 0;
    };

    template <typename Order, typename Base>
        struct IAgentForOrderImpl : Base, IAgentForOrder<Order>
        {
            template <class T>
                IAgentForOrderImpl(T const & x)
                    :   Base(x)
                {}

            DECLARE_BASE(IAgentForOrderImpl);

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

    template <typename AgentPtr, typename Base>
        struct WithLinkToAgent : Base 
        {
            template <typename T>
                WithLinkToAgent(T const & x)
                    :   Base  (boost::get<0>(x))
                    ,   agent_(boost::get<1>(x))
                {}

        typedef WithLinkToAgent base; // for derived typenamees

        template <typename OtherOrder>
            void onMatched(PriceVolume const & x, OtherOrder const &o)
        {
            Base::onMatched(x,o);
            agent_->onOrderPartiallyFilled(self(), x);
            if (filled())
                agent_->onOrderFilled(self());
        }

        void onCancelled()
        {
            Base::onCancelled();
            agent_->onOrderCancelled(self());
        }

        private:
            AgentPtr  agent_;
        };
}

#endif