#include "stdafx.h"
#include "catch.hpp"

#include <marketsim/scheduler.h>
#include <marketsim/rng.h>

#include <marketsim/order/base.h>

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/market_order_factory.h>
#include <marketsim/agent/signal_trader.h>
#include <marketsim/agent/signal.h>

namespace marketsim {
namespace {

    template <Side SIDE>
        struct MarketT : MarketOrderBase<SIDE, MarketT<SIDE> >
        {
            template <class X>
                MarketT(Volume v, X) : base(v) {}
        };

    struct SignalTraderTester :
        SignalTrader<rng::constant<Volume>, 
            MarketOrderFactory<MarketT<Buy>, MarketT<Sell>, 
                LinkToOrderBook<SignalTraderTester*, 
                    AgentBase<SignalTraderTester> > > >
    {
        SignalTraderTester()
            :   base(boost::make_tuple(dummy, rng::constant<Volume>(1), 0.7))
            ,   signal_(rng::constant<Time>(1.), boost::ref(*self()), self())
            ,   delta_(0)
            ,   processed_(0)
        {
            setOrderBook(this);
        }

        bool processOrder(MarketT<Buy> const & x)
        {
            processed_ -= x.volume;
            return true;
        }

        bool processOrder(MarketT<Sell> const & x)
        {
            processed_ += x.volume;
            return true;
        }

        double operator () () 
        {
            return delta_;
        }

        void setDelta(double d) { delta_ = d; }

        Volume getProcessed() const { return processed_; }

    private:
        typedef Signal<rng::constant<Time>, SignalTraderTester&, SignalTraderTester*>   signal_t;

        signal_t    signal_;
        Volume      processed_;
        double      delta_;
    };

    TEST_CASE("signal_trader", "Testing that a signal tester sends an order when threshold is surpassed")
    {
        SignalTraderTester    trader;

        scheduler().workTill(1.5);
        REQUIRE(trader.getProcessed() == 0);

        trader.setDelta(1.);

        scheduler().workTill(3.5);
        REQUIRE(trader.getProcessed() == 2);

        trader.setDelta(-1.);

        scheduler().workTill(4.5);
        REQUIRE(trader.getProcessed() == 3);

        scheduler().workTill(5.5);
        REQUIRE(trader.getProcessed() == 3);
        
        scheduler().workTill(7.5);
        REQUIRE(trader.getProcessed() == 1);

        scheduler().reset();
    }

}}