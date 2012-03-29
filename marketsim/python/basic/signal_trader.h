#ifndef _marketsim_python_basic_signal_trader_h_included
#define _marketsim_python_basic_signal_trader_h_included

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/agent/signal.h>
#include <marketsim/agent/signal_trader.h>

namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

   //--------------------------------------------------------------- Agents

   namespace agent 
   {
        struct Signal_Trader :
            IAgentForMarketOrderImpl    < order::MarketT<Buy>, order::MarketT<Sell>, 
            PnL_Quantity_History_InDeque<
            SignalTrader                < py_value<VolumeF>, 
            MarketOrderFactory          < order::MarketT<Buy>, order::MarketT<Sell>, 
            LinkToOrderBook             < boost::intrusive_ptr<OrderBook>, 
            PyRefCounted                <
            AgentBase                   < Signal_Trader, IRefCounted> 
            > > > > > >
        {
            Signal_Trader(boost::intrusive_ptr<OrderBook> book, py_object volumeDist, double threshold)
                :   base(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            volumeDist, 
                            threshold
                        ))
            {}

            static void py_register(const char * name)
            {
                class_<
                    Signal_Trader, 
                    boost::intrusive_ptr<Signal_Trader>,
                    boost::noncopyable
                > 
                c(name, no_init);

                base::py_visit(c);

                register_3<Signal_Trader, boost::intrusive_ptr<OrderBook>, py_object, double>(c);
            }
        };

        struct Signal : 
            marketsim::agent::Signal < py_value<Time>, py_value<double>, boost::intrusive_ptr<Signal_Trader>, 
            PyRefCounted             < IRefCounted >
            >
        {
            Signal(boost::intrusive_ptr<Signal_Trader> trader, py_object updateDist, py_object signalDist)
                :   base(updateDist, signalDist, trader)
            {}

            static void py_register(const char * name)
            {
                class_<
                    Signal, 
                    boost::intrusive_ptr<Signal>,
                    boost::noncopyable
                > c(name, no_init);

                base::py_visit(c);

                register_3<Signal, boost::intrusive_ptr<Signal_Trader>, py_object, py_object>(c);
            }
        };
   }
}}

#endif
