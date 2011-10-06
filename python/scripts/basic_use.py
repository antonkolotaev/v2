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

def py_NoiseTrader(book, intervalDistr, volumeDistr):

        trader = MarketOrderTrader(book)

        def wakeUp():
            v = int(volumeDistr())
            if v > 0:
                trader.sendBuyOrder(v)
            if v < 0:
                trader.sendSellOrder(v)

        py_timer(intervalDistr, wakeUp)

        return trader

def py_FV_Trader(book, intervalDistr, volumeDistr, FV):

        trader = MarketOrderTrader(book)

        def wakeUp():
            if not book.asks.empty() and not book.bids.empty():
                midPrice2 = book.asks.bestPrice() + book.bids.bestPrice()
                v = int(volumeDistr())
                if midPrice2 > FV*2:
                    trader.sendSellOrder(v)
                if midPrice2 < FV*2:
                    trader.sendBuyOrder(v)

        py_timer(intervalDistr, wakeUp)

        return trader

class py_Signal():

    def __init__(self, intervalDistr, deltaDistr):
        self.listeners = set()
        self.signal = 0.

        def wakeUp():
            self.signal += deltaDistr()
            for x in self.listeners:
                x(self.signal)

        py_timer(intervalDistr, wakeUp)

def py_Signal_Trader(book, signal, threshold, volumeDistr):

    trader = MarketOrderTrader(book)

    def wakeUp(signal_value):
        if signal_value > threshold:
            trader.sendSellOrder(int(volumeDistr()))
        if signal_value < -threshold:
            trader.sendBuyOrder(int(volumeDistr()))

    signal.listeners.add(wakeUp)

    return trader

def py_LiquidityProvider_Sell(book, sendOrderInterval, cancelOrderInterval, priceDistr, volumeDistr, initialPrice):

        seller = LimitOrderTrader_Sell(book)

        def sendAnOrder():
            if book.asks.lastTradedVolume() == 0:
                price = initialPrice
            else:
                price = book.asks.lastTradedPrice()
            price = book.ceilPrice(price + priceDistr())
            volume = int(volumeDistr())
            seller.sendOrder(price, volume)

        def cancelAnOrder():
            n = seller.ordersIssuedCount()
            if n > 0:
                idx = uniform_smallint(0,n-1)()
                seller.cancelOrder(idx)

        py_timer(sendOrderInterval, sendAnOrder)
        py_timer(cancelOrderInterval, cancelAnOrder)

        return seller

def py_LiquidityProvider_Buy(book, sendOrderInterval, cancelOrderInterval, priceDistr, volumeDistr, initialPrice):

        buyer = LimitOrderTrader_Buy(book)

        def sendAnOrder():
            if book.bids.lastTradedVolume() == 0:
                price = initialPrice
            else:
                price = book.bids.lastTradedPrice()
            price = book.floorPrice(price - priceDistr())
            volume = int(volumeDistr())
            seller.sendOrder(price, volume)

        def cancelAnOrder():
            n = buyer.ordersIssuedCount()
            if n > 0:
                idx = uniform_smallint(0,n-1)()
                buyer.cancelOrder(idx)

        py_timer(sendOrderInterval, sendAnOrder)
        py_timer(cancelOrderInterval, cancelAnOrder)

        return buyer

scheduler = PyScheduler()

py_signal = py_Signal(exponential(1.), normal(0., 1.))


def YYY():
    print "Time = ", scheduler.currentTime()

ttt = py_timer(exponential(1.), YYY)

book = OrderBook()
book.tickSize = 2
seller = LiquidityProvider_Sell(book, exponential(1.), exponential(.1), exponential(10.), exponential(0.1), 500)
buyer = LiquidityProvider_Buy(book, exponential(1.), exponential(.1), exponential(10.), exponential(0.1), 400)

py_signal_trader = py_Signal_Trader(book, py_signal, 0.7, exponential(0.1))

py_noisetrader = py_NoiseTrader(book, exponential(1.), normal(0., 10.))
py_fv_trader = py_FV_Trader(book, exponential(1.), exponential(.1), 500)

py_seller = py_LiquidityProvider_Sell(book, exponential(1.), exponential(1.), normal(1., 5.), exponential(.1), 500)
py_buyer = py_LiquidityProvider_Buy(book, exponential(1.), exponential(1.), normal(1., 5.), exponential(.1), 500)

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

trader_sell = LimitOrderTrader_Sell(book)
trader_sell.sendOrder(400, 100)

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

for a in book_history.getHistory():
    print a

#print list(book_history.getHistory())

