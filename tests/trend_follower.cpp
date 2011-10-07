
#include "catch.hpp"


#include <marketsim/rng.h>

#include <marketsim/order/base.h>

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/market_order_factory.h>
#include <marketsim/agent/trend_follower.h>

namespace marketsim {
namespace {

    template <Side SIDE>
        struct MarketT : MarketOrderBase<SIDE, MarketT<SIDE> >
        {
            template <typename X>
                MarketT(Volume v, X) : base(v) {}
        };

    struct TrendFollowerTester :
                TrendFollower       <rng::constant<Time>, rng::constant<Volume>, 
                MarketOrderFactory  <MarketT<Buy>, MarketT<Sell>, 
                LinkToOrderBook     <TrendFollowerTester*, 
                AgentBase           <TrendFollowerTester>
            > > >
    {
        TrendFollowerTester()
            :   base(
                    boost::make_tuple(
                        boost::make_tuple(dummy,this), 
                        rng::constant<Time>(1.), 
                        rng::constant<Volume>(1))
                    )
            ,   ask_(0), bid_(0)
            ,   processed_(0)
        {
        }

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

        Price bestPriceImpl(sell_tag) const { return ask_; }
        Price bestPriceImpl(buy_tag) const { return bid_; }

        template <Side SIDE>  Price bestPrice(side_tag<SIDE> s = side_tag<SIDE>()) const { return bestPriceImpl(s); }

        void setBid(Price b) { bid_ = b; }
        void setAsk(Price a) { ask_ = a; }

        Volume getProcessed() const { return processed_; }

    private:
        Volume      processed_;
        Price       bid_, ask_;
    };

    TEST_CASE("trend_follower", "Checking that trend follower follows the trend")
    {
        Scheduler               scheduler;
        TrendFollowerTester     trader;

        trader.setBid(95); trader.setAsk(105);

        scheduler.workTill(1.5);
        REQUIRE(trader.getProcessed() == 0);

        scheduler.workTill(2.5);
        REQUIRE(trader.getProcessed() == 0);

        trader.setAsk(110);

        scheduler.workTill(3.5);
        REQUIRE(trader.getProcessed() == -1);

        trader.setBid(10);

        scheduler.workTill(4.5);
        REQUIRE(trader.getProcessed() == 0);
    }

}}