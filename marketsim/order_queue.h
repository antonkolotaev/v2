#ifndef _marketsim_order_queue_h_included_
#define _marketsim_order_queue_h_included_

#include <queue>
#include <algorithm>

#include <marketsim/common_types.h>

namespace marketsim 
{
    // requires: 
    //      Order is a pointer-like type
    //      order->cancelled() returns true iff the order is useless and thus may be removed
    template <class Order>
        struct OrderQueue : protected std::priority_queue<Order, std::vector<Order>, typename ordered_by_price<Order>::type>
    {
		typedef typename ordered_by_price<Order>::type comparer_type;

        typedef 
            std::priority_queue<Order, std::vector<Order>, typename ordered_by_price<Order>::type>
            base; 

		OrderQueue() {}

		OrderQueue(Dummy) {}

        using base::reference;
        using base::push;
        using base::top;
		using base::empty;
		using base::value_type;

        void pop() // we suppose that the order on top is valid
        { 
            base::pop(); 
            make_valid();
        }

        struct Cancalled {
            bool operator () (Order const & q) const {
                return q->cancelled();
            }
        };

        template <class T>
            void onOrderCancelled(T x)
            {
                make_valid();
            }

        std::vector<Order>  getSorted() 
        {
            std::vector<Order> v(c.begin(), c.end());
            std::sort_heap(v.begin(), v.end(), this->comp);
            v.erase(std::remove_if(v.begin(), v.end(), Cancalled()), v.end());
            std::reverse(v.begin(), v.end());
            return v;
        }

        std::vector<Price> getSortedPrices()
        {
            std::vector<Price>  p;
            BOOST_FOREACH(Order o, getSorted())
            {
                p.push_back(o->price);
            }
            return p;
        }

        std::vector<Volume> getSortedVolumes()
        {
            std::vector<Volume>  p;
            BOOST_FOREACH(Order o, getSorted())     // TODO: use transform_view
            {
                p.push_back(o->volume);
            }
            return p;
        }

        friend std::ostream & operator << (std::ostream & out, OrderQueue const & q)
        {
            out << "[";
            std::copy(q.c.begin(), q.c.end(), std::ostream_iterator<Order>(out, "; "));
            out << "]";
            return out;
        }

		template <class ValueType, class RanIt, class Predicate, class Function>
			struct ForKeysBetterThan
		{
			ForKeysBetterThan(ValueType const & x, RanIt const & it, size_t N, Predicate const & p, Function & F)
				:	x(x), it(it), p(p), N(N), F(F)
			{
				Walk(1);
			}

		private:
			void Walk(size_t Idx)
			{
				assert(Idx >= 1);
				assert(Idx < N);

				if (!p(it[Idx], x))
				{
					if (!F(it[Idx]))
						return;

					if (2*Idx < N)
					{
						Walk(2*Idx);

						if (2*Idx+1 < N)
							Walk(2*Idx+1);
					}
				}
			}

		private:
			ValueType const & x;
			RanIt	  const & it;
			size_t	  const   N;
			Predicate const & p;
			Function		& F;
		};

		struct AccVolume 
		{
			template <class Order>
				bool operator () (Order const & order)
			{
				result += order->volume;				
				return true;
			}

			AccVolume() : result(0) {}

			Volume result;
		};

		Volume volumeForBetterPrices(Price p) const 
		{
			AccVolume acc_1;
			ForKeysBetterThan<Price, value_type const*, comparer_type, AccVolume>(p, &c[0]-1, c.size()+1, comp, acc_1);	
			return acc_1.result;
		}

		Volume getBestVolume() const 
		{
			return empty() ? 0 : volumeForBetterPrices(top()->price);
		}

		struct IdxComparer
		{
			IdxComparer(comparer_type const & comp, value_type const * ptr) : comp(comp), ptr(ptr) {}

			bool operator () (size_t lhs, size_t rhs)  const
			{
				return comp(ptr[lhs], ptr[rhs]);
			}

		private:
			comparer_type const & comp;
			value_type    const * ptr; 
		};

		struct IndicesQueue : std::priority_queue<size_t, std::vector<size_t>, IdxComparer>
		{
			IndicesQueue(IdxComparer const & comp) :  
				std::priority_queue<size_t, std::vector<size_t>, IdxComparer>(comp)
			{}

			void reset()
			{
				c.resize(0);
			}
		};

		std::vector<PriceVolume>	getBestN(int N) const 
		{
			std::vector<PriceVolume>	result;
			result.reserve(N);
			getBestN(N, std::back_inserter(result));
			return result;
		}

		template <class OutputIterator>
			void getBestN(int N, OutputIterator out) const 
		{
			PriceVolume		last(-1,-1);
			int  			sent_1 = 0;

			value_type const * before_first = &c[0] - 1;
			size_t size_1 = c.size() + 1;

			IdxComparer idx_comparer(comp, before_first);
			// we use static in order to avoid excessive memory allocations ==> this function is not thread-safe!
			static IndicesQueue	indices(idx_comparer);
			indices.reset();
			
			if (!c.empty())
				indices.push(1);

			while (!indices.empty())
			{
				size_t idx = indices.top();

				if (last.price != before_first[idx]->price)
				{
					if (sent_1 && last.volume)
						*out++ = last;

					if (sent_1 == N)
						return;

					last.price = before_first[idx]->price;
					last.volume = before_first[idx]->volume;
					++sent_1;
				}
				else
					last.volume += before_first[idx]->volume;

				indices.pop();

				if (2*idx < size_1)
				{
					indices.push(2*idx);
					if (2*idx+1 < size_1)
						indices.push(2*idx+1);
				}
			}
			if (sent_1 && last.volume)
				*out++ = last;
		}

		template <class Order>
			void onPartiallyFilled(Order const & order, PriceVolume const & trade)
			{}

		DECLARE_ARROW(OrderQueue);

		void getHandler(); // to be defined in derived classes if needed
    private:
        void make_valid()
        {
            while (!base::empty())
            {
                reference t = base::top();
                if (t->cancelled())
                    base::pop();
                else break;
            }
        }
    };

	template <class Handler, class Base>
		struct OnQueueTopChanged : Base
	{
		OnQueueTopChanged() {}

		template <class T> OnQueueTopChanged(T const & x) 
			: Base	  (boost::get<0>(x))
			, handler_(boost::get<1>(x))
		{}

		typedef OnQueueTopChanged base; // for derived classes

		using Base :: value_type;

		template <class T>
			void push(T x)
			{
				Base::push(x);
				if (!comp(x, top()))
					handler_(self());
			}

		template <class T>
			void onOrderCancelled(T x)
			{
				bool better = !comp(x, top());
				Base::onOrderCancelled(x);
				if (better)
					handler_(self());		// TODO: use self() and introduce derived_t
			}

		void pop() 
		{
			Base::pop();
			handler_(self());
		}

		template <class Order>
			void onPartiallyFilled(Order const & order, PriceVolume const & trade)
		{
			Base::onPartiallyFilled(order, trade);
			handler_(self());
		}

		Handler const & getHandler(Handler *) const { return handler_; }
		Handler       & getHandler(Handler *)       { return handler_; }

		using Base::getHandler;

	private:
		Handler			handler_;	
	};
}

#endif