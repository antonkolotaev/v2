#ifndef _marketsim_python_basic_market_order_trader_h_included
#define _marketsim_python_basic_market_order_trader_h_included

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/agent/market_order_factory.h>

namespace marketsim {
namespace basic {

    using namespace boost::python;

    typedef boost::python::object   py_object;

   //--------------------------------------------------------------- Agents

   namespace agent 
   {
       using namespace marketsim::agent;

        struct MarketOrderTrader :
            IAgentForMarketOrderImpl< order::MarketT<Buy>, order::MarketT<Sell>, 
            OnPartiallyFilled       < py_callback,
            PnL_Holder              <
            Quantity_Holder         <
            MarketOrderFactory      < order::MarketT<Buy>, order::MarketT<Sell>, 
            LinkToOrderBook         < boost::intrusive_ptr<OrderBook>,
            PyRefCounted            <
            AgentBase               < MarketOrderTrader, IRefCounted
            > > > > > > > >
        {
            MarketOrderTrader(boost::intrusive_ptr<OrderBook> book)
                :   base(boost::make_tuple(boost::make_tuple(dummy, book), dummy))
            {}

            static void py_register()
            {
                class_<
                    MarketOrderTrader, 
                    boost::intrusive_ptr<MarketOrderTrader>,
                    boost::noncopyable 
                > 
                c("MarketOrderTrader", no_init);

                base::py_visit(c);

                register_1<MarketOrderTrader, boost::intrusive_ptr<OrderBook> >(c);
            }
        };
   }
}}

#endif
