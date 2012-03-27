#ifndef _marketsim_fast_fv_trader_h_included
#define _marketsim_fast_fv_trader_h_included

#include <marketsim/agent/base.h>
#include <marketsim/agent/fundamental_value_trader.h>
#include <marketsim/agent/market_order_factory.h>
#include <marketsim/agent/link_to_orderbook.h>

#include "PnL_Quantity_history_in_deque.h"

namespace marketsim {
namespace fast {

    namespace agent 
    {
        using namespace marketsim::agent;

        struct FV_Trader :
            PnL_Quantity_History_InDeque<
            FundamentalValueTrader  <rng::exponential<Time>, rng::exponential<VolumeF>, 
            MarketOrderFactory      <order::MarketT<Buy, FV_Trader*>, order::MarketT<Sell, FV_Trader*>, 
            LinkToOrderBook         <boost::intrusive_ptr<OrderBook>, 
            PyRefCounted            <
            AgentBase               <FV_Trader, IRefCounted
            > > > > > >
        {
            FV_Trader(boost::intrusive_ptr<OrderBook> book, Price FV, Time creationTime, VolumeF meanVolume)
                :   base(
                boost::make_tuple(
                boost::make_tuple(dummy, book), 
                rng::exponential<Time>(creationTime), 
                rng::exponential<VolumeF>(meanVolume), 
                FV)
                )
            {}

            static void py_register(const char * name)
            {
                using namespace boost::python;

                class_<
                    FV_Trader, 
                    boost::intrusive_ptr<FV_Trader>,
                    boost::noncopyable
                > 
                c(name, no_init);

                base::py_visit(c);
                register_4<FV_Trader, boost::intrusive_ptr<OrderBook>, Price, Time, VolumeF>(c);
            }
        };
    }
}}

MARKETSIM_PY_REGISTER_NAME(marketsim::fast::agent::FV_Trader, "FV_Trader");

#endif