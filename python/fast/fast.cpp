#include <boost/python.hpp>
#include <marketsim/ref_counted.h>
#include <marketsim/py_ref_counted.h>

#include <list>
#include <string>

typedef marketsim::IRefCounted IPyRefCounted;
namespace py = boost::python;

#include "market_order.h"
#include "limit_order.h"
#include "order_book.h"
#include "liquidity_provider.h"
#include "fv_trader.h"
#include "signal_trader.h"
#include "signal.h"
#include "noise_trader.h"

MARKETSIM_PY_REGISTER_NAME(marketsim::Scheduler, "Scheduler");

BOOST_PYTHON_MODULE(fast)
{
    marketsim::py_register<IPyRefCounted>();
    marketsim::registerClassesInPython();
}
