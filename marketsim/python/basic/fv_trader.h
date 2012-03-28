#ifndef _marketsim_python_basic_fv_trader_h_included
#define _marketsim_python_basic_fv_trader_h_included

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/agent/market_order_factory.h>
#include <marketsim/agent/fundamental_value_trader.h>
#include <marketsim/agent/agent_for_order.h>

namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

   //--------------------------------------------------------------- Agents

   namespace agent 
   {

        struct FV_Trader :
            OnPartiallyFilled       < py_callback,
            PnL_Quantity_History_InDeque<
            FundamentalValueTrader  < py_value<Time>, py_value<VolumeF>, 
            MarketOrderFactory      < order::MarketT<Buy, FV_Trader*>, order::MarketT<Sell, FV_Trader*>, 
            LinkToOrderBook         < boost::intrusive_ptr<OrderBook>, 
            PyRefCounted            <
            AgentBase               < FV_Trader, IRefCounted
            > > > > > > >
        {
            FV_Trader(boost::intrusive_ptr<OrderBook> book, Price FV, py_object intervalDist, py_object volumeDist)
                :   base(
                        boost::make_tuple(
                            boost::make_tuple(
                                boost::make_tuple(dummy, book), 
                                intervalDist, 
                                volumeDist, 
                                FV),
                            dummy)
                        )
            {}

            static void py_register(const char * name)
            {
                class_<
                    FV_Trader, 
                    boost::intrusive_ptr<FV_Trader>,
                    boost::noncopyable 
                > 
                c(name, no_init);

                base::py_visit(c);

                register_4<FV_Trader, boost::intrusive_ptr<OrderBook>, Price, py_object, py_object>(c);
            }
        };
   }
}}

#endif
