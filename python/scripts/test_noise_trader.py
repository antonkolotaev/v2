from marketsim import Side
from marketsim.scheduler import world
from marketsim.order import *
from marketsim.order_queue import *
from marketsim.trader import *

book = OrderBook(tickSize=.001)

counter = [0]

def side():
    counter[0] = 1 - counter[0]
    return counter[0]

trader = NoiseTrader(book, sideDistr=side, volumeDistr=(lambda:10), creationIntervalDistr=(lambda:1))

world.workTill(1.5)

book.process(LimitOrderBuy(90, 20))
book.process(LimitOrderSell(110, 20))

world.workTill(2.5)

assert book.bids.best.volume == 10
assert trader.PnL == +10*90

world.workTill(3.5)

assert book.asks.best.volume == 10
assert trader.PnL == +10*90 - 10*110
