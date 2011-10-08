
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

    template <Side SIDE> struct AgentT;

    namespace order {

        using namespace  marketsim::order;

        template <Side SIDE>
            struct MarketT : MarketOrderBase<SIDE, MarketT<SIDE> >
            {
                template <typename X>
                    MarketT(Volume v, X) : base(v) {}
            };

        template <Side SIDE>
            struct LimitT : 
                    WithLinkToAgent < AgentT<SIDE>*,
                    InPool          < PlacedInPool, 
                    LimitOrderBase  < SIDE, 
                    LimitT          < SIDE
                    > > > > 
            {
                LimitT(PriceVolume const &x, object_pool<LimitT> * h, AgentT<SIDE> * agent) 
                    :   base(boost::make_tuple(boost::make_tuple(x, h), agent))
                {}
            };

    }
            
    typedef order::LimitT<Buy>     LimitBuy;
    typedef order::LimitT<Sell>    LimitSell;

   typedef boost::intrusive_ptr<LimitBuy>   LimitBuyPtr;
   typedef boost::intrusive_ptr<LimitSell>  LimitSellPtr;

   struct BestPriceAndVolume 
   {
	   typedef PriceVolume	ValueType;

	   template <typename T> 
			static ValueType getValue(T x) 
			{
				return PriceVolume(x->top()->getPrice(), x->getBestVolume());
			}
   };

   struct OrderBook : marketsim::OrderBook<
	   OrderQueue<LimitBuyPtr>, 
	   OnQueueTopChanged<history::Collector<BestPriceAndVolume, history::InDeque<PriceVolume> >, 
			OrderQueue<LimitSellPtr> 
	   > 
   >    
   {
       std::deque<history::TimeStamped<PriceVolume> > const & getHistory() 
	   {
		   orderQueue(sell_tag()).getHandler((history::Collector<BestPriceAndVolume, history::InDeque<PriceVolume> >*)0).flush();
		   return orderQueue(sell_tag()).getHandler((history::Collector<BestPriceAndVolume, history::InDeque<PriceVolume> >*)0).getHistory();
	   }
   };

   template <Side SIDE>
    struct AgentT :
        OnPartiallyFilled   < history::Collector<PnL, history::InDeque<Price> >,
        OnPartiallyFilled   < history::Collector<Quantity, history::InDeque<Volume> >,
        OnPartiallyFilled   < history::Collector<PnL, history::InFile>,  
        PnL_Holder          <
        Quantity_Holder     <
        LinkToOrderBook     < OrderBook*, 
        PrivateOrderPool    < order::LimitT<SIDE>, 
        AgentBase           < AgentT<SIDE> > 
        > > > > > > > 
    {
		AgentT(OrderBook* book) : base(
            boost::make_tuple(
                boost::make_tuple(
                    boost::make_tuple(
                        boost::make_tuple(dummy, book), 
                        "history.log"), 
                    dummy), 
                dummy)) 
        {}

		template <typename TAG>
            std::deque<history::TimeStamped<typename TAG::ValueType> > const & getHistory() 
			{
				getHandler((history::Collector<TAG, history::InDeque<typename TAG::ValueType> >*)0).flush();
				return getHandler((history::Collector<TAG, history::InDeque<typename TAG::ValueType> >*)0).getHistory();
			}

        order::LimitT<SIDE> * sendOrder(Price p, Volume v)
        {
            order::LimitT<SIDE> * o = base::createOrder(pv(p,v));

            base::processOrder(o);

            return o;
        }
    };

    inline order::MarketT<Buy> buy(Volume v)
    {
        return order::MarketT<Buy>(v, dummy);
    }

    typedef history::TimeStamped<Price> PnLHistoryPiece;
    typedef history::TimeStamped<Volume>  QuantityHistoryPiece;

    TEST_CASE("PnL_test", "checking that PnL history works well")
    {
        Scheduler       scheduler;
        OrderBook       book;
        AgentT<Sell>    trader(&book);

        trader.setQuantity(0);

        trader.sendOrder(100, 3);
        trader.sendOrder(102, 5);
        trader.sendOrder(105, 7);

        std::deque<history::TimeStamped<PriceVolume> > book_history = book.getHistory();

		assert(book_history[0].time == 0);
		assert(book_history[0].value == PriceVolume(100, 3));
		assert(book_history.size() == 1);

		assert(trader.getPnL() == 0);
        assert(trader.getHistory<PnL>().empty());
        assert(trader.getQuantity() == 0);
        assert(trader.getHistory<Quantity>().empty());

        scheduler.workTill(1.5);

        book.processOrder(buy(1));

		book_history = book.getHistory();

		assert(book_history[1].time == 1.5);
		assert(book_history[1].value == PriceVolume(100, 2));
		assert(book_history.size() == 2);

        assert(trader.getPnL() == 100);
        assert(trader.getQuantity() == -1);
        assert(!trader.getHistory<PnL>().empty());
        assert(!trader.getHistory<Quantity>().empty());
        assert(trader.getHistory<PnL>().back() == PnLHistoryPiece(1.5, 100));
        assert(trader.getHistory<Quantity>().back() == QuantityHistoryPiece(1.5, -1));

        scheduler.workTill(3.5);

        book.processOrder(buy(3));

		book_history = book.getHistory();

		assert(book_history[2].time == 3.5);
		assert(book_history[2].value == PriceVolume(102, 4));
		assert(book_history.size() == 3);

		assert(trader.getPnL() == 100 + 200 + 102);
        assert(trader.getQuantity() == -4);
        assert(trader.getHistory<PnL>().size() == 2);
        assert(trader.getHistory<PnL>().back() == PnLHistoryPiece(3.5, 100 + 200 + 102));
        assert(trader.getHistory<Quantity>().size() == 2);
        assert(trader.getHistory<Quantity>().back() == QuantityHistoryPiece(3.5, -4));
    }

}}