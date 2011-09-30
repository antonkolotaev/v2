#ifndef _marketsim_agent_market_order_factory_h_included_
#define _marketsim_agent_market_order_factory_h_included_

namespace marketsim
{
    template <typename MarketBuy, typename MarketSell, typename Base>
        struct MarketOrderFactory : Base 
        {
            template <typename T> MarketOrderFactory(T const x) : Base(x) {}

            template <typename T>
                MarketBuy  createOrder(T const &x, buy_tag) 
                { 
                    return MarketBuy(x, self()); 
                }

            template <typename T>
                MarketSell createOrder(T const &x, sell_tag) 
                { 
                    return MarketSell(x, self()); 
                }

            void sendBuyOrder(Volume x)
            {
                self()->processOrder(createOrder(x, buy_tag()));
            }

            void sendSellOrder(Volume x)
            {
                self()->processOrder(createOrder(x, sell_tag()));
            }

#ifdef MARKETSIM_BOOST_PYTHON

            template <typename T>
                static void py_visit(T & c)
                {
                    c.def("sendBuyOrder", &MarketOrderFactory::sendBuyOrder);
                    c.def("sendSellOrder",&MarketOrderFactory::sendSellOrder);
                }

#endif
        };

}

#endif