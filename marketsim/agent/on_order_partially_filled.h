#ifndef _marketsim_agent_on_order_partially_filled_h_included_
#define _marketsim_agent_on_order_partially_filled_h_included_

namespace marketsim
{
    template <class TAG, class HistoryFunc, class Base>
        struct OnPartiallyFilled : TAG::base<Base>::type
    {
        typedef typename TAG::base<Base>::type          RealBase;
        typedef typename HistoryFunc::apply<TAG>::type  History;

        template <class T>
            OnPartiallyFilled(T const & x)
                :   RealBase(x) {}

        typedef OnPartiallyFilled base; // for derived classes

        typename History::HistoryStorage const & getHistory(TAG)
        {
            return history_.getHistory();
        }

        void recordHistory(TAG)
        {
            history_.recordHistory();
        }

        using TAG::base<Base>::type::getHistory; 
        using TAG::base<Base>::type::recordHistory; 

        template <class Order>
            void onOrderPartiallyFilled(Order order, PriceVolume const & x)
        {
            RealBase::onOrderPartiallyFilled(order, x);
            history_.update(self());            
        }
    private:
        History     history_;
    };

}

#endif