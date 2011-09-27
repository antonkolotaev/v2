#ifndef _marketsim_order_link_to_agent_h_included_
#define _marketsim_order_link_to_agent_h_included_

namespace marketsim 
{
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