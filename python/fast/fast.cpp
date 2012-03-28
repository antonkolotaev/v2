#include <boost/python.hpp>
#include <marketsim/ref_counted.h>
#include <marketsim/python/py_ref_counted.h>
#include <marketsim/python/registration.h>

#include <list>
#include <string>

typedef marketsim::IRefCounted IPyRefCounted;
namespace py = boost::python;

#include <marketsim/python/fast/market_order.h>
#include <marketsim/python/fast/limit_order.h>
#include <marketsim/python/fast/order_book.h>
#include <marketsim/python/fast/liquidity_provider.h>
#include <marketsim/python/fast/fv_trader.h>
#include <marketsim/python/fast/signal_trader.h>
#include <marketsim/python/fast/signal.h>
#include <marketsim/python/fast/noise_trader.h>

MARKETSIM_PY_REGISTER_NAME(marketsim::Scheduler, "Scheduler");

BOOST_PYTHON_MODULE(fast)
{
    marketsim::py_register<IPyRefCounted>();
    marketsim::registerClassesInPython();
}
