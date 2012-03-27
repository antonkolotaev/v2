import sys
sys.path.append(r'D:\marketsimulator\DevAnton\v2\Debug')

from marketsim.basic import *

scheduler = Scheduler()
assert (scheduler.currentTime()==0.)
scheduler.advance(10.)
assert(scheduler.currentTime() == 10.)
scheduler.reset()
assert (scheduler.currentTime()==0.)


book = OrderBook()
assert(book.asks.empty())
assert(book.bids.empty())

seller = LimitOrderTrader_Sell(book)
buyer = LimitOrderTrader_Buy(book)

seller.sendOrder(90, 100)
assert(book.asks.empty() == False)
assert(book.asks.bestPrice() == 90)
assert(book.asks.bestVolume() == 100)

buyer.sendOrder(80, 100)
assert(book.bids.empty() == False)
assert(book.bids.bestPrice() == 80)
assert(book.bids.bestVolume() == 100)

buyer.sendOrder(95, 10)
assert(book.asks.empty() == False)
assert(book.asks.bestPrice() == 90)
assert(book.asks.bestVolume() == 90)

assert(buyer.assetsAvailable == 10)
assert(buyer.PnL == -900)

assert(seller.assetsAvailable == -10)
assert(seller.PnL == 900)

l_seller = LiquidityProvider_Sell(book, exponential(1.), exponential(2.), normal(10.,10.), normal(100., 10.), 100)
l_buyer = LiquidityProvider_Buy(book, exponential(1.), exponential(2.), normal(10.,10.), normal(100., 10.), 100)

scheduler.advance(100)

for i in l_seller.PnL_Quantity_History(): print i

for i in l_buyer.PnL_Quantity_History(): print i


"""
book = OrderBook()
LiquidityProvider_Sell(book, 1., 1., 10., 100., 100)
LiquidityProvider_Buy(book, 1., 1., 10., 100., 100)

FV_Trader(book, 120, 1., 100.)

signal = Signal(Signal_Trader(book, 100, 0.2), 1., 1.)

Noise_Trader(book, 1., 100)

scheduler.advance(100)

for i in book.asks.history(): print i
"""

