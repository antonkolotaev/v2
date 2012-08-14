from marketsim.order import *
from marketsim.order_queue import *

asks = Asks()

a12 = LimitOrderSell(12, 100)
a10 = LimitOrderSell(10, 100)
a15 = LimitOrderSell(15, 100)
asks.push(a12)
asks.push(a10)
asks.push(a15)

L = set(asks.withPricesBetterThen(12))
assert a10 in L
assert a12 in L
assert a15 not in L


b = LimitOrderBuy(9, 100)
assert asks.matchWith(b) == False
assert b.volume == 100
assert b.PnL == 0
assert asks.best.price == 10
assert asks.best.volume == 100

b = LimitOrderBuy(10, 30)
assert asks.matchWith(b) == True
assert b.volume == 0
assert b.PnL == -10*30
assert asks.best.price == 10
assert asks.best.volume == 70
assert a10.PnL == +10*30

b = LimitOrderBuy(10, 80)
assert asks.matchWith(b) == False
assert b.volume == 10
assert b.PnL == -10*70
assert asks.best.price == 12
assert asks.best.volume == 100
assert a10.PnL == +10*100

b = LimitOrderBuy(20, 180)
assert asks.matchWith(b) == True
assert b.volume == 0
assert a12.PnL == +12*100
assert a15.PnL == +15*80
assert b.PnL == -12*100 -15*80
assert asks.best.price == 15
assert asks.best.volume == 20

b = LimitOrderBuy(20, 180)
assert asks.matchWith(b) == False
assert b.volume == 160
assert b.PnL == -20*15
assert a15.PnL == +15*100
assert asks.empty

