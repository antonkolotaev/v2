#ifndef _marketsim_order_queue_on_queue_top_changed_h_included_
#define _marketsim_order_queue_on_queue_top_changed_h_included_

namespace marketsim {
namespace order_queue 
{
    /// Base class for order queue which allows to define a reaction on the queue top change
    template <typename Handler, typename Base>
		struct OnQueueTopChanged : Base
	{
		OnQueueTopChanged() {}

		DECLARE_BASE(OnQueueTopChanged);

		using Base :: value_type;

        /// If an order is inserted into the head of the queue, call the handler
		template <typename T>
			void push(T x)
			{
				Base::push(x);
				if (!comp(x, this->top()))
					handler_(this->self());
			}

        /// If the order on the top of the queue is cancelled, call the handler
		template <typename T>
			void onOrderCancelled(T x)
			{
				bool better = !comp(x, this->top());
				Base::onOrderCancelled(x);
				if (better)
					handler_(this->self());		
			}

        /// if the best order is popped from the queue, the handler is called
		void pop() 
		{
			Base::pop();
			handler_(this->self());
		}

        /// onPartiallyFilled is called when volume of the best order is changed,
        /// so let's call the handler
		template <typename Order>
			void onPartiallyFilled(Order const & order, PriceVolume const & trade)
		{
			Base::onPartiallyFilled(order, trade);
			handler_(this->self());
		}

		Handler const & getHandler(Handler *) const { return handler_; }
		Handler       & getHandler(Handler *)       { return handler_; }

		using Base::getHandler;

#ifdef MARKETSIM_BOOST_PYTHON
        template <class T>  
            static void py_visit(T & c)
            {
                Base::py_visit(c);
                c.def_readonly("on_top_changed", &OnQueueTopChanged::handler_);
            }

#endif

	private:
		Handler			handler_;	

        OnQueueTopChanged(OnQueueTopChanged const &);
	};

}}

#endif
