#ifndef _marketsim_agent_market_order_factory_in_pool_h_included_
#define _marketsim_agent_market_order_factory_in_pool_h_included_

#include <marketsim/agent/order_pool.h>

namespace marketsim {
namespace agent 
{
    
    template <
        typename MarketBuy,     // type of market buy orders to create. it should have a ctor(T const&, AgentType*)
        typename MarketSell,    // type of market sell orders to create. it should have a ctor(T const&, AgentType*)
        typename Base
    >
        struct MarketOrderFactoryInSharedPool 
            :   Base 
            ,   private SharedOrderPool<MarketBuy, derived_is<typename Base::derived_t> >
            ,   private SharedOrderPool<MarketSell,derived_is<typename Base::derived_t> >
        {
            template <typename T> MarketOrderFactoryInSharedPool(T const x) : Base(x) {}

            /// creates a buy market order
            /// \param T usually it is order volume
            template <typename T>
                MarketBuy*  createOrder(T const &x, buy_tag) 
                { 
                    return SharedOrderPool<MarketBuy, derived_is<typename Base::derived_t> >::createOrder(x); 
                }

            /// creates a sell market order
            /// \param T usually it is order volume
            template <typename T>
                MarketSell* createOrder(T const &x, sell_tag) 
                { 
                    return SharedOrderPool<MarketSell, derived_is<typename Base::derived_t> >::createOrder(x); 
                }

            /// a shortcut for sending market orders of the given volume
            template <Side SIDE>
                void sendMarketOrder(Volume x)
            {
                this->self()->processOrder(createOrder(x, side_tag<SIDE>()));
            }

#ifdef MARKETSIM_BOOST_PYTHON

            template <typename T>
                static void py_visit(T & c)
                {
                    c.def("sendBuyOrder", &MarketOrderFactoryInSharedPool::sendMarketOrder<Buy>);
                    c.def("sendSellOrder",&MarketOrderFactoryInSharedPool::sendMarketOrder<Sell>);
                }

#endif
        };
}}