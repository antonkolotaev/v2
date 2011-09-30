import sys
sys.path.append(r'D:\dev_1\marketsimulator\v2\Debug')

from marketsim.fast import *

scheduler = Scheduler()

book = OrderBook()
book.tickSize = 2
seller = LiquidityProvider_Sell(book, 1., .1, 10., 0.1, 500)
buyer = LiquidityProvider_Buy(book, 1., .1, 10., 0.1, 400)

fv_trader = FV_Trader(book, 600, 1., 0.1)
#seller.sendOrder(100, 5)

s_trader = Signal_Trader(book, 0.1, 0.005)
signal = Signal(s_trader, 1., .2)

noise_trader = Noise_Trader(book, 1., 0.1)

book.bids.recordHistory(False)

for i in range(10):
    scheduler.workTill(scheduler.currentTime() + 10)
    L = book.asks.bestOrders(1)
    R = book.bids.bestOrders(1)
    print scheduler.currentTime(), signal.signalValue, str(R), str(L)

for a in book.asks.history():
    print a

for a in book.bids.history():
    print a

