
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
        struct MarketT : 
            order::MarketOrderBase  <SIDE,
            derived_is              <
            MarketT                 <SIDE> 
            > >
        {
            typedef 
                order::MarketOrderBase  <SIDE,
                derived_is              <
                MarketT                 <SIDE> 
                > >
                base;

            template <typename X>
                MarketT(Volume v, X) : base(v) {}
        };

    struct SignalTraderTester :
            agent::SignalTrader        <rng::constant<Volume>, 
            agent::MarketOrderFactory  <MarketT<Buy>, MarketT<Sell>, 
            agent::LinkToOrderBook     <SignalTraderTester*, 
            agent::AgentBase           <SignalTraderTester> 
            > > >
    {
        SignalTraderTester()
            :   base(boost::make_tuple(boost::make_tuple(dummy, this), rng::constant<Volume>(1), 0.7))
            ,   signal_(rng::constant<Time>(1.), boost::ref(*self()), self())
            ,   delta_(0)
            ,   processed_(0)
        {}

        bool processOrder(MarketT<Buy> const & x)
        {
            processed_ -= x.getVolume();
            return true;
        }

        bool processOrder(MarketT<Sell> const & x)
        {
            processed_ += x.getVolume();
            return true;
        }

        double operator () () 
        {
            return delta_;
        }

        void setDelta(double d) { delta_ = d; }

        Volume getProcessed() const { return processed_; }

    private:
        typedef marketsim::agent::Signal<rng::constant<Time>, SignalTraderTester&, SignalTraderTester*>   signal_t;

        signal_t    signal_;
        Volume      processed_;
        double      delta_;
    };

    TEST_CASE("signal_trader", "Testing that a signal tester sends an order when threshold is surpassed")
    {
        Scheduler               scheduler;
        SignalTraderTester      trader;

        scheduler.workTill(1.5);
        assert(trader.getProcessed() == 0);

        trader.setDelta(1.);

        scheduler.workTill(3.5);
        assert(trader.getProcessed() == 2);

        trader.setDelta(-1.);

        scheduler.workTill(4.5);
        assert(trader.getProcessed() == 3);

        scheduler.workTill(5.5);
        assert(trader.getProcessed() == 3);
        
        scheduler.workTill(7.5);
        assert(trader.getProcessed() == 1);
    }

}}