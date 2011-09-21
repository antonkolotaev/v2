#include "stdafx.h"
#include "catch.hpp"


#include <marketsim/object_pool.h>
#include <marketsim/order_book.h>

#include <marketsim/order/base.h>
#include <marketsim/order/link_to_agent.h>
#include <marketsim/order/in_pool.h>

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/order_pool.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>

#include <marketsim/history.h>

namespace marketsim {
namespace {

    template <Side SIDE>
        struct MarketT : MarketOrderBase<SIDE, MarketT<SIDE> >
        {
            template <class X>
                MarketT(Volume v, X) : base(v) {}
        };

    template <Side SIDE> struct AgentT;

    template <Side SIDE>
        struct LimitT : 
            WithLinkToAgent<AgentT<SIDE>*,
                InPool<PlacedInPool, 
                    LimitOrderBase<SIDE, 
                        LimitT<SIDE> > > > 
        {
            LimitT(PriceVolume const &x, object_pool<LimitT> * h, AgentT<SIDE> * agent) 
                :   base(boost::make_tuple(boost::make_tuple(x, h), agent))
            {}
        };
            
   typedef LimitT<Buy>     LimitBuy;
   typedef LimitT<Sell>    LimitSell;

   typedef boost::intrusive_ptr<LimitBuy>   LimitBuyPtr;
   typedef boost::intrusive_ptr<LimitSell>  LimitSellPtr;

   typedef OrderBook<LimitBuyPtr, LimitSellPtr>    OrderBook;

   template <Side SIDE>
    struct AgentT :
        OnPartiallyFilled<PnL, History,
            OnPartiallyFilled<Quantity, History,
                LinkToOrderBook<OrderBook*, 
                    PrivateOrderPool<LimitT<SIDE>, 
                        AgentBase<AgentT<SIDE> > > > > > 
    {
        AgentT() : base(dummy) {}

        LimitT<SIDE> * sendOrder(Price p, Volume v)
        {
            LimitT<SIDE> * order = base::createOrder(pv(p,v));

            base::processOrder(order);

            return order;
        }
    };

    inline MarketT<Buy> buy(Volume v)
    {
        return MarketT<Buy>(v, dummy);
    }

    TEST_CASE("PnL_test", "checking that PnL history works well")
    {
        AgentT<Sell>    trader;

        OrderBook   book;
        trader.setOrderBook(&book);
        trader.setQuantity(0);

        trader.sendOrder(100, 3);
        trader.sendOrder(102, 5);
        trader.sendOrder(105, 7);

        REQUIRE(trader.getPnL() == 0);
        REQUIRE(trader.getHistory(PnL()).empty());
        REQUIRE(trader.getQuantity() == 0);
        REQUIRE(trader.getHistory(Quantity()).empty());

        scheduler().workTill(1.5);

        book.processOrder(buy(1));

        REQUIRE(trader.getPnL() == 100);
        REQUIRE(trader.getQuantity() == -1);
        REQUIRE(!trader.getHistory(PnL()).empty());
        REQUIRE(!trader.getHistory(Quantity()).empty());
        REQUIRE(trader.getHistory(PnL()).back() == PnLHistoryPiece(1.5, 100));
        REQUIRE(trader.getHistory(Quantity()).back() == QuantityHistoryPiece(1.5, -1));

        scheduler().workTill(3.5);

        book.processOrder(buy(3));

        REQUIRE(trader.getPnL() == 100 + 200 + 102);
        REQUIRE(trader.getQuantity() == -4);
        REQUIRE(trader.getHistory(PnL()).size() == 2);
        REQUIRE(trader.getHistory(PnL()).back() == PnLHistoryPiece(3.5, 100 + 200 + 102));
        REQUIRE(trader.getHistory(Quantity()).size() == 2);
        REQUIRE(trader.getHistory(Quantity()).back() == QuantityHistoryPiece(3.5, -4));

        scheduler().reset();
    }

}}