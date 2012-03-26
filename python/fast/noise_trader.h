#ifndef _marketsim_fast_noise_trader_h_included
#define _marketsim_fast_noise_trader_h_included

#include <marketsim/agent/base.h>
#include <marketsim/agent/noise_trader.h>
#include <marketsim/agent/market_order_factory.h>
#include <marketsim/agent/link_to_orderbook.h>

#include "PnL_Quantity_history_in_deque.h"

namespace marketsim {
namespace fast {

    namespace agent 
    {
        using namespace marketsim::agent;

        struct Noise_Trader :
            PnL_Quantity_History_InDeque<
            NoiseTrader         < rng::exponential<Time>,     rng::exponential<VolumeF>,
            MarketOrderFactory  < order::MarketT<Buy,Noise_Trader*>, order::MarketT<Sell,Noise_Trader*>, 
            LinkToOrderBook     < boost::intrusive_ptr<OrderBook>, 
            PyRefCounted        <
            AgentBase           < Noise_Trader, IRefCounted
            > > > > > >
        {
            Noise_Trader(boost::intrusive_ptr<OrderBook> book, Time meanInterval, VolumeF meanVolume)
                :   base(
                boost::make_tuple(
                boost::make_tuple(dummy, book), 
                rng::exponential<Time>(meanInterval), 
                rng::exponential<VolumeF>(meanVolume)
                ))
            {}

            static void py_register(const char * name)
            {
                using namespace boost::python;

                class_<
                    Noise_Trader, 
                    boost::intrusive_ptr<Noise_Trader>,
                    bases<IRefCounted>,
                    boost::noncopyable
                >
                c(name, no_init);

                base::py_visit(c);
                register_3<Noise_Trader, boost::intrusive_ptr<OrderBook>, Time, VolumeF>(c);
            }
        };
    }
}}

MARKETSIM_PY_REGISTER_NAME(marketsim::fast::agent::Noise_Trader, "Noise_Trader");

#endif