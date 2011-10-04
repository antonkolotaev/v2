import sys
sys.path.append(r'D:\dev_1\marketsimulator\v2\Debug')

from marketsim.basic import *

class PyScheduler(Scheduler):

    holder = 0

    def __init__(self):
        super(PyScheduler,self).__init__()
        PyScheduler.holder = set()

    def __del__(self):
        L = list(PyScheduler.holder)
        for s in L:
            s.cancel()
        PyScheduler.holder = 0


class py_event:
    def __init__(self, dt, handler):
        self.event = Event(dt, handler, self.remove)
        PyScheduler.holder.add(self)

    def cancel(self):
        self.event.cancel()

    def remove(self):
        self.event = 0
        PyScheduler.holder.remove(self)
        print "removed"

class py_timer:
    def __init__(self, intervals, handler):
        self.timer = Timer(intervals, handler, self.remove)
        PyScheduler.holder.add(self)

    def cancel(self):
        self.timer.cancel()

    def remove(self):
        PyScheduler.holder.remove(self)
        self.timer = 0
        print "timer removed"

class py_Trader(MarketOrderTrader):

    def __init__(self,book):
        super(py_Trader,self).__init__(book)

class py_NoiseTrader(MarketOrderTrader):

    def __init__(self, book, intervalDistr, volumeDistr):

        super(py_NoiseTrader, self).__init__(book)

        def wakeUp():
            v = int(volumeDistr())
            if v > 0:
                self.sendBuyOrder(v)
            if v < 0:
                self.sendSellOrder(v)

        py_timer(intervalDistr, wakeUp)


class py_FV_Trader(MarketOrderTrader):

    def __init__(self, book, intervalDistr, volumeDistr, FV):

        super(py_FV_Trader, self).__init__(book)

        def wakeUp():
            if not book.asks.empty() and not book.bids.empty():
                midPrice2 = book.asks.bestPrice() + book.bids.bestPrice()
                v = int(volumeDistr())
                if midPrice2 > FV*2:
                    self.sendSellOrder(v)
                if midPrice2 < FV*2:
                    self.sendBuyOrder(v)

        py_timer(intervalDistr, wakeUp)

scheduler = PyScheduler()

def YYY():
    print "Time = ", scheduler.currentTime()

ttt = py_timer(exponential(1.), YYY)

book = OrderBook()
book.tickSize = 2
seller = LiquidityProvider_Sell(book, exponential(1.), exponential(.1), exponential(10.), exponential(0.1), 500)
buyer = LiquidityProvider_Buy(book, exponential(1.), exponential(.1), exponential(10.), exponential(0.1), 400)

py_noisetrader = py_NoiseTrader(book, exponential(1.), normal(0., 10.))
py_fv_trader = py_FV_Trader(book, exponential(1.), exponential(.1), 500)

history = HistoryInDeque_PnL_Quantity()

history.recording = not history.recording
history.recording = not history.recording

fv_trader = FV_Trader(book, 600, exponential(1.), exponential(0.1))
#seller.sendOrder(100, 5)

def H(p):
    print p.PnL

def F(p):
    print p.bestOrders(3)

book_history = HistoryInDeque_BestPriceAndVolume()

def P() : print "Event called!"

event = py_event(1., P)
#event.cancel();

#print "Holder = ", holder
#schedule(2., P)

book.asks.on_top_changed.add(book_history)

fv_trader.on_partially_filled.add(history)

s_trader = Signal_Trader(book, exponential(0.1), 0.005)
signal = Signal(s_trader, exponential(1.), normal(0., .2))

noise_trader = Noise_Trader(book, exponential(1.), exponential(0.1))

book.bids.recordHistory(False)

for i in range(10):
    scheduler.workTill(scheduler.currentTime() + 10)
    L = book.asks.bestOrders(1)
    R = book.bids.bestOrders(1)
    print scheduler.currentTime(), signal.signalValue, str(R), str(L)

ttt.cancel()

#print "Holder = ", holder

fv_trader.on_partially_filled.remove(H)
print "No PnL here"

seller.sendOrder(500, 100)
fv_trader.sendBuyOrder(100)

fv_trader.on_partially_filled.add(H)
print "print PnL here"

seller.sendOrder(500, 100)
fv_trader.sendBuyOrder(100)

#for a in book.asks.history():
#    print a

for a in book.bids.history():
    print a

#print list(book_history.getHistory())

