#ifndef _marketsim_order_link_to_agent_h_included_
#define _marketsim_order_link_to_agent_h_included_

namespace marketsim {
namespace order {
    /// Base class for orders used to notify an agent created the order about order's events:
    /// - order partial filling:    void onOrderPartiallyFilled(Order*, PriceVolume const & trade);
    /// - order complete filling:   void onOrderFilled(Order*);
    /// - order cancellation:       void onOrderCancelled(Order*);
    template <typename AgentPtr, typename Base>
        struct WithLinkToAgent : Base 
        {
            /// 0-th argument is an initializer for the base class
            /// 1-th argument is a pointer to the agent to be notified about changes in the order
            template <typename T>
                WithLinkToAgent(T const & x)
                    :   Base  (boost::get<0>(x))
                    ,   agent_(boost::get<1>(x))
                {}

        DECLARE_BASE(WithLinkToAgent);

        /// Called when order is matched with another one
        /// \param x trade (Price,Volume)
        /// \param o reference to the other order (usually not used)
        template <typename OtherOrder>
            void onMatched(PriceVolume const & x, OtherOrder const &o)
        {
            Base::onMatched(x,o);
            agent_->onOrderPartiallyFilled(this->self(), x);
            if (this->filled())
                agent_->onOrderFilled(this->self());
        }

        /// Called when order is cancelled
        void onCancelled()
        {
            Base::onCancelled();
            agent_->onOrderCancelled(this->self());
        }

        private:
            AgentPtr  agent_;
        };
}}

#endif
