#include <boost/python.hpp>
#include <list>
#include <set>
//#define MARKETSIM_BOOST_PYTHON

#include <marketsim/python/py_ref_counted.h>
#include <marketsim/python/registration.h>

#include <marketsim/object_pool.h>
#include <marketsim/order_book.h>

#include <marketsim/agent/base.h>
#include <marketsim/agent/link_to_orderbook.h>
#include <marketsim/agent/order_pool.h>
#include <marketsim/agent/PnL.h>
#include <marketsim/agent/quantity.h>
#include <marketsim/agent/on_order_partially_filled.h>
#include <marketsim/agent/liquidity_provider.h>
#include <marketsim/agent/canceller.h>
#include <marketsim/agent/fundamental_value_trader.h>
#include <marketsim/agent/market_order_factory.h>
#include <marketsim/agent/signal.h>
#include <marketsim/agent/signal_trader.h>
#include <marketsim/agent/noise_trader.h>
#include <marketsim/agent/agent_for_order.h>

#include <marketsim/history.h>
#include <marketsim/python/callback.h>

#include "with_history_in_deque.h"
#include "PnL_Quantity_history_in_deque.h"

#include <marketsim/python/basic/orders.h>
#include <marketsim/python/basic/order_book.h>
#include <marketsim/python/basic/limit_order_trader.h>
#include <marketsim/python/basic/liquidity_provider.h>
#include <marketsim/python/basic/market_order_trader.h>
#include <marketsim/python/basic/fv_trader.h>
#include <marketsim/python/basic/signal_trader.h>
#include <marketsim/python/basic/noise_trader.h>
#include <marketsim/python/scheduled_event.h>
#include <marketsim/python/timer.h>

MARKETSIM_PY_REGISTER(marketsim::basic::ScheduledEvent);
MARKETSIM_PY_REGISTER(marketsim::basic::Timer);

MARKETSIM_PY_REGISTER_NAME(marketsim::Scheduler, "Scheduler");
MARKETSIM_PY_REGISTER_NAME(marketsim::basic::OrderBook, "OrderBook");

MARKETSIM_PY_REGISTER(marketsim::basic::agent::LimitOrderTraderT<marketsim::Ask>);
MARKETSIM_PY_REGISTER(marketsim::basic::agent::LimitOrderTraderT<marketsim::Bid>);

MARKETSIM_PY_REGISTER(marketsim::basic::agent::LiquidityProviderT<marketsim::Ask>);
MARKETSIM_PY_REGISTER(marketsim::basic::agent::LiquidityProviderT<marketsim::Bid>);

MARKETSIM_PY_REGISTER(marketsim::basic::agent::MarketOrderTrader);
MARKETSIM_PY_REGISTER_NAME(marketsim::basic::agent::FV_Trader, "FV_Trader");
MARKETSIM_PY_REGISTER_NAME(marketsim::basic::agent::Signal_Trader, "Signal_Trader");
MARKETSIM_PY_REGISTER_NAME(marketsim::basic::agent::Signal, "Signal");
MARKETSIM_PY_REGISTER_NAME(marketsim::basic::agent::Noise_Trader, "Noise_Trader");

BOOST_PYTHON_MODULE(basic)
{
	using namespace marketsim::basic;
    using marketsim::py_register;

    py_register<marketsim::IRefCounted>();
    marketsim::registerClassesInPython();

    py_register<marketsim::PriceVolume>();

    py_register<marketsim::rng::exponential<> >();
    py_register<marketsim::rng::constant<> >();
    py_register<marketsim::rng::gamma<> >();
    py_register<marketsim::rng::lognormal<> >();
    py_register<marketsim::rng::normal<> >();
    py_register<marketsim::rng::uniform_01<> >();
    py_register<marketsim::rng::uniform_real<> >();
    py_register<marketsim::rng::uniform_smallint<> >();

    py_register<marketsim::py_callback>();

    py_register<marketsim::history::CollectInDeque<marketsim::py_PnL_Quantity> >();
    py_register<marketsim::history::CollectInDeque<marketsim::order_queue::py_BestPriceAndVolume> >();
}

