from marketsim import Side
from marketsim.scheduler import world
from marketsim.order import *
from marketsim.order_queue import *
from marketsim.trader import *

book = OrderBook(tickSize=.001)

trader = LiquidityProvider(book,
                           side=Side.Sell,
                           creationIntervalDistr=(lambda: 1),
                           priceDistr=(lambda: 0.5),
                           volumeDistr=(lambda: 10),
                           defaultValue=128)

assert book.asks.empty

world.workTill(1.5)

assert book.asks.best.price == 64
assert book.asks.best.volume == 10

world.workTill(2.5)

assert book.asks.best.price == 32
assert book.asks.best.volume == 10

book.process(MarketOrderBuy(5))
assert trader.PnL == 32*5

book.process(MarketOrderBuy(10))
assert trader.PnL == 32*10 + 64*5

assert book.asks.best.price == 64
assert book.asks.best.volume == 5

canceller = Canceller(cancellationIntervalDistr=(lambda: .2), choiceFunc=(lambda N: 0))
trader.on_order_sent.add(canceller.process)

world.workTill(3.05)

assert book.asks.best.price == 32
assert book.asks.best.volume == 10

world.workTill(3.15)

assert book.asks.best.price == 64
assert book.asks.best.volume == 5
