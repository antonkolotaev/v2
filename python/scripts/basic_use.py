import sys
sys.path.append(r'D:\dev_1\marketsimulator\v2\Debug')

from marketsim.basic import *

scheduler = Scheduler()

book = OrderBook()
book.tickSize = 2
seller = LiquidityProvider_Sell(book, exponential(1.), exponential(.1), exponential(10.), exponential(0.1), 500)
buyer = LiquidityProvider_Buy(book, exponential(1.), exponential(.1), exponential(10.), exponential(0.1), 400)

def F(p):
    pass

fv_trader = FV_Trader(book, 600, exponential(1.), exponential(0.1))
#seller.sendOrder(100, 5)

def H(p):
    print p.PnL

fv_trader.on_partially_filled.add(H)

s_trader = Signal_Trader(book, exponential(0.1), 0.005)
signal = Signal(s_trader, exponential(1.), normal(0., .2))

noise_trader = Noise_Trader(book, exponential(1.), exponential(0.1))

book.bids.recordHistory(False)

for i in range(10):
    scheduler.workTill(scheduler.currentTime() + 10)
    L = book.asks.bestOrders(1)
    R = book.bids.bestOrders(1)
    print scheduler.currentTime(), signal.signalValue, str(R), str(L)

fv_trader.on_partially_filled.remove(H)
print "No PnL here"

seller.sendOrder(500, 100)
fv_trader.sendBuyOrder(100)

fv_trader.on_partially_filled.add(H)
print "print PnL here"

seller.sendOrder(500, 100)
fv_trader.sendBuyOrder(100)

for a in book.asks.history():
    print a

for a in book.bids.history():
    print a

