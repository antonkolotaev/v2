#ifndef _marketsim_python_basic_noise_trader_h_included
#define _marketsim_python_basic_noise_trader_h_included

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/agent/market_order_factory.h>
#include <marketsim/agent/noise_trader.h>
#include <marketsim/agent/agent_for_order.h>

namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

   //--------------------------------------------------------------- Agents

   namespace agent 
   {
        struct Noise_Trader :
            PnL_Quantity_History_InDeque<
            NoiseTrader         < py_value<Time>, py_value<VolumeF>,
            MarketOrderFactory  < order::MarketT<Buy,Noise_Trader*>, order::MarketT<Sell,Noise_Trader*>, 
            LinkToOrderBook     < boost::intrusive_ptr<OrderBook>, 
            PyRefCounted        <
            AgentBase           < Noise_Trader, IRefCounted
            > > > > > >
        {
            Noise_Trader(boost::intrusive_ptr<OrderBook> book, py_object interval, py_object meanVolume)
                :   base(
                        boost::make_tuple(
                            boost::make_tuple(dummy, book), 
                            interval, 
                            meanVolume
                        ))
            {}

            static void py_register(const char * name)
            {
                class_<
                    Noise_Trader, 
                    boost::intrusive_ptr<Noise_Trader>,
                    boost::noncopyable
                > 
                c(name, no_init);

                base::py_visit(c);

                register_3<Noise_Trader, boost::intrusive_ptr<OrderBook>, py_object, py_object>(c);
            }
        };

   }
}}

#endif
