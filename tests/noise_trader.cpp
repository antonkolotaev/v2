
#include "catch.hpp"

#include <marketsim/scheduler.h>
#include <marketsim/rng.h>

#include <marketsim/order/base.h>

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/noise_trader.h>
#include <marketsim/agent/market_order_factory.h>

namespace marketsim {
namespace {

    template <Side SIDE>
        struct MarketT : MarketOrderBase<SIDE, MarketT<SIDE> >
        {
            template <typename X>
                MarketT(Volume v, X) : base(v) {}
        };


    struct NoiseTraderTester :
        NoiseTrader<rng::constant<Time>, NoiseTraderTester&, 
            MarketOrderFactory<MarketT<Buy>, MarketT<Sell>, 
                LinkToOrderBook<NoiseTraderTester*, 
                    AgentBase<NoiseTraderTester> > > >
    {
        NoiseTraderTester()
            :   base(boost::make_tuple(dummy, rng::constant<double>(1.), boost::ref(*self())))
            ,   v_(1)
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

        Volume operator () () 
        {
            return v_;
        }

        void setVolume(Volume v) { v_ = v; }

        Volume getProcessed() const { return processed_; }

    private:
        Volume  v_;

        Volume  processed_;
    };

    TEST_CASE("noise_trader", "checking that noise trader sends orders")
    {
        NoiseTraderTester  tester;

        scheduler().workTill(3.5);

        REQUIRE(tester.getProcessed() == 3);

        tester.setVolume(-3);

        scheduler().workTill(5.5);

        REQUIRE(tester.getProcessed() == -3);

        scheduler().reset();
    }
}}