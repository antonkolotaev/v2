#ifndef _marketsim_agent_market_order_factory_h_included_
#define _marketsim_agent_market_order_factory_h_included_

namespace marketsim
{
    template <class MarketBuy, class MarketSell, class Base>
        struct MarketOrderFactory : Base 
        {
            template <class T> MarketOrderFactory(T const x) : Base(x) {}

            template <class T>
                MarketBuy  createOrder(T const &x, buy_tag) 
                { 
                    return MarketBuy(x, self()); 
                }

            template <class T>
                MarketSell createOrder(T const &x, sell_tag) 
                { 
                    return MarketSell(x, self()); 
                }
        };

}

#endif