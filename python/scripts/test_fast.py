import sys
#sys.path.append(r'D:\marketsimulator\DevAnton\v2\Debug')

from marketsim.fast import *

scheduler = Scheduler()
book = OrderBook()
LiquidityProvider_Sell(book, 1., 1., 10., 100., 100)
LiquidityProvider_Buy(book, 1., 1., 10., 100., 100)

FV_Trader(book, 120, 1., 100.)

signal = Signal(Signal_Trader(book, 100, 0.2), 1., 1.)

Noise_Trader(book, 1., 100)

scheduler.advance(100)

for i in book.asks.history(): print i


