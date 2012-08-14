import random
from marketsim.scheduler import world
from marketsim import Side
from marketsim.order import *

class LiquidityProvider(object):

   def __init__(self,
                orderBook,
                side=Side.Sell,
                orderFactoryT=LimitOrderT,
                defaultValue=100,
                creationIntervalDistr=(lambda: random.expovariate(1.)),
                priceDistr=(lambda: random.lognormvariate(0., .1)),
                volumeDistr=(lambda: random.expovariate(.1))):

      orderFactory = orderFactoryT(side)

      self._PnL = 0
      self._side = side
      self.on_order_sent = set()

      def onOrderMatched(order, other, (price, volume)):
          self._PnL += price*volume

      def wakeUp():
         queue = orderBook.queue(side)
         currentPrice = queue.best.price if not queue.empty else defaultValue
         price = currentPrice * priceDistr()
         volume = int(volumeDistr())
         order = orderFactory(price,volume)
         order.on_matched.add(onOrderMatched)
         orderBook.process(order)
         for x in self.on_order_sent: x(order)

      world.process(creationIntervalDistr, wakeUp)

   @property
   def PnL(self):
      return self._PnL if self._side==Side.Sell else -self._PnL

class Canceller(object):

   def __init__(self,
                cancellationIntervalDistr=(lambda: random.expovariate(1.)),
                choiceFunc=lambda N: random.randint(0,N-1)):

      self._elements = []

      def wakeUp():
         while self._elements <> []:
            N = len(self._elements)
            idx = choiceFunc(N)
            e = self._elements[idx]
            if e.empty or e.cancelled:
               if e <> self._elements[-1]:
                  self._elements[idx] = self._elements[-1]
               self._elements.pop()
            else:
               e.cancel()
               return

      world.process(cancellationIntervalDistr, wakeUp)

   def process(self, order):
      self._elements.append(order)
