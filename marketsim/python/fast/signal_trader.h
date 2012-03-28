#ifndef _marketsim_fast_signal_trader_h_included
#define _marketsim_fast_signal_trader_h_included

#include <marketsim/agent/base.h>
#include <marketsim/agent/signal_trader.h>
#include <marketsim/agent/market_order_factory.h>
#include <marketsim/agent/link_to_orderbook.h>

#include "PnL_Quantity_history_in_deque.h"

namespace marketsim {
namespace fast {

    namespace agent 
    {
        using namespace marketsim::agent;

        struct Signal_Trader :
            PnL_Quantity_History_InDeque<
            SignalTrader        <rng::exponential<VolumeF>, 
            MarketOrderFactory  <order::MarketT<Buy, Signal_Trader*>, order::MarketT<Sell, Signal_Trader*>, 
            LinkToOrderBook     <boost::intrusive_ptr<OrderBook>,
            PyRefCounted        <
            AgentBase           <Signal_Trader, IRefCounted> 
            > > > > >
        {
            Signal_Trader(boost::intrusive_ptr<OrderBook> book, VolumeF meanVolume, double threshold)
                :   base(
                boost::make_tuple(
                boost::make_tuple(dummy, book), 
                rng::exponential<VolumeF>(meanVolume), 
                threshold
                ))
            {}

            static void py_register(const char * name)
            {
                using namespace boost::python;

                class_<
                    Signal_Trader, 
                    boost::intrusive_ptr<Signal_Trader>,
                    bases<IRefCounted>,
                    boost::noncopyable
                > 
                c(name, no_init);

                base::py_visit(c);
                register_3<Signal_Trader, boost::intrusive_ptr<OrderBook>, VolumeF, double>(c);
            }
        };
    }
}}

MARKETSIM_PY_REGISTER_NAME(marketsim::fast::agent::Signal_Trader, "Signal_Trader");

#endif