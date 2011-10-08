#ifndef _marketsim_agent_market_order_factory_h_included_
#define _marketsim_agent_market_order_factory_h_included_

namespace marketsim {
namespace agent 
{
    /// Base class for agents sending market orders
    /// It serves as a factory for market orders
    /// Since they are to be executed immediately we pass it by value but not as reference
    /// It also defines two functions for convenience for sending orders
    /// TODO: use meta class defining market order types
    template <
        typename MarketBuy,     // type of market buy orders to create. it should have a ctor(T const&, AgentType*)
        typename MarketSell,    // type of market sell orders to create. it should have a ctor(T const&, AgentType*)
        typename Base
    >
        struct MarketOrderFactory : Base 
        {
            template <typename T> MarketOrderFactory(T const x) : Base(x) {}

            /// creates a buy market order
            /// \param T usually it is order volume
            template <typename T>
                MarketBuy  createOrder(T const &x, buy_tag) 
                { 
                    return MarketBuy(x, self()); 
                }

            /// creates a sell market order
            /// \param T usually it is order volume
            template <typename T>
                MarketSell createOrder(T const &x, sell_tag) 
                { 
                    return MarketSell(x, self()); 
                }

            /// a shortcut for sending market orders of the given volume
            template <Side SIDE>
                void sendMarketOrder(Volume x)
            {
                self()->processOrder(createOrder(x, side_tag<SIDE>()));
            }

#ifdef MARKETSIM_BOOST_PYTHON

            template <typename T>
                static void py_visit(T & c)
                {
                    c.def("sendBuyOrder", &MarketOrderFactory::sendMarketOrder<Buy>);
                    c.def("sendSellOrder",&MarketOrderFactory::sendMarketOrder<Sell>);
                }

#endif
        };

}}

#endif