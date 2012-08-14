import heapq

class _EventHandler(object):
   def __init__(self, handler):
      self._handler = handler
      self._cancelled = False

   def __call__(self):
      self._handler()

   def cancel(self):
      self._cancelled = True

   @property
   def cancelled(self):
      return self._cancelled

   def __repr__(self):
      return "("+repr(self._handler) + ("-> Cancelled" if self.cancelled else "") + ")"

class Scheduler(object):

   def __init__(self):
      self.reset()

   def reset(self):
      self._elements = []
      self._currentTime = 0.

   def __repr__(self):
      return "(t=" + str(self.currentTime) + ": " + repr(self._elements) + ")"

   @property
   def currentTime(self):
      return self._currentTime

   def schedule(self, actionTime, handler):
      assert actionTime >= self.currentTime
      eh = _EventHandler(handler)
      event = (actionTime, eh)
      heapq.heappush(self._elements, event)
      return eh.cancel

   def scheduleAfter(self, dt, handler):
      self.schedule(self.currentTime + dt, handler)

   def workTill(self, limitTime):
      while (self._elements <> [] and self._elements[0][0] < limitTime):
         (actionTime, eh) = heapq.heappop(self._elements)
         if not eh.cancelled:
            self._currentTime = actionTime
            eh()
      self._currentTime = limitTime

   def advance(self, dt):
      self.workTill(self.currentTime + dt)

   def process(self, intervalFunc, handler):
       def h():
          handler()
          self.scheduleAfter(intervalFunc(), h)
       self.scheduleAfter(intervalFunc(), h)

world = Scheduler()