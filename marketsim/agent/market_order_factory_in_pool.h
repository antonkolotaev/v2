#ifndef _marketsim_agent_market_order_factory_in_pool_h_included_
#define _marketsim_agent_market_order_factory_in_pool_h_included_

#include <marketsim/agent/order_pool.h>

namespace marketsim {
namespace agent 
{
    
    template <
        typename MarketBuyPtr,     // type of market buy orders to create. it should have a ctor(T const&, AgentType*)
        typename MarketSellPtr,    // type of market sell orders to create. it should have a ctor(T const&, AgentType*)
        typename Base
    >
        struct MarketOrderFactoryInPool 
            :   Base 
            ,   private SharedOrderPool<
                    MarketBuyPtr, 
                    derived_is<typename Base::derived_t> 
                >
            ,   private SharedOrderPool<
                    MarketSellPtr,
                    derived_is<typename Base::derived_t> 
                >
        {
            template <typename T> MarketOrderFactoryInPool(T const x) : Base(x) {}

            typedef typename boost::iterator_value<MarketBuyPtr> ::type  MarketBuy;
            typedef typename boost::iterator_value<MarketSellPtr>::type  MarketSell;

            /// creates a buy market order
            /// \param T usually it is order volume
            template <typename T>
                MarketBuyPtr  createOrder(T const &x, buy_tag) 
                { 
                    return SharedOrderPool<MarketBuyPtr, derived_is<typename Base::derived_t> >::createOrder(x); 
                }

            /// creates a sell market order
            /// \param T usually it is order volume
            template <typename T>
                MarketSellPtr createOrder(T const &x, sell_tag) 
                { 
                    return SharedOrderPool<MarketSellPtr, derived_is<typename Base::derived_t> >::createOrder(x); 
                }

                using Base::derived_t;
                using Base::self;

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
                    c.def("sendBuyOrder", &MarketOrderFactoryInPool::sendMarketOrder<Buy>);
                    c.def("sendSellOrder",&MarketOrderFactoryInPool::sendMarketOrder<Sell>);
                }

#endif
        };
}}

#endif