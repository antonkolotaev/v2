#ifndef _marketsim_order_queue_h_included_
#define _marketsim_order_queue_h_included_

#include <queue>
#include <algorithm>
#include <boost/type_traits/remove_pointer.hpp>

#include <marketsim/common_types.h>

namespace marketsim 
{
    // requires: 
    //      Order is a pointer-like type
    //      order->cancelled() returns true iff the order is useless and thus may be removed
    template <typename Order>
        struct OrderQueue : protected std::priority_queue<Order, std::vector<Order>, typename ordered_by_price<Order>::type>
    {
    private:
        OrderQueue(OrderQueue const &);
    public:
		typedef typename ordered_by_price<Order>::type comparer_type;

        typedef 
            std::priority_queue<Order, std::vector<Order>, typename ordered_by_price<Order>::type>
            base; 

		OrderQueue() {}

		OrderQueue(Dummy) {}

        using base::reference;
        using base::push;
        using base::top;
		//using base::empty;
		using base::value_type;

        void pop() // we suppose that the order on top is valid
        { 
            base::pop(); 
            make_valid();
        }

        bool empty() const 
        {
            return base::empty();
        }

        struct Cancalled {
            bool operator () (Order const & q) const {
                return q->cancelled();
            }
        };

        template <typename T>
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

		template <typename ValueType, typename RanIt, typename Predicate, typename Function>
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
			template <typename Order>
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

        Price getBestPrice() const 
        {
            /// !!!! We should use a free meta function in order to get access to the real order type
            typedef typename Order::element_type  E;
            return empty() ? E::worstPrice() : top()->price;
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

        template <typename PoolPtr>
        struct BelongsTo {
            BelongsTo(PoolPtr a) : a(a) {}
            template <typename OrderPtr> bool operator () (OrderPtr o) const {
                return o->is_my_pool(a);
            }
            PoolPtr a;
        };

        template <typename PoolPtr>
            void remove_all_pool_orders(PoolPtr p) 
            {
                c.erase(std::remove_if(c.begin(),c.end(),BelongsTo<PoolPtr>(p)), c.end());
                std::make_heap(c.begin(), c.end(), comp);
            }

		template <typename OutputIterator>
			void getBestN(int N, OutputIterator out) const 
		{
			PriceVolume		last(-1,-1);
			int  			sent_1 = 0;

            if (c.empty())
                return;

			value_type const * before_first = &c[0] - 1;
			size_t size_1 = c.size() + 1;

			IdxComparer     idx_comparer(comp, before_first);
			IndicesQueue	indices(idx_comparer);
			
			if (!c.empty())
				indices.push(1);

			while (!indices.empty())
			{
				size_t idx = indices.top();

				if (last.price != before_first[idx]->price)
				{
                    if (before_first[idx]->volume)
                    {
                        if (sent_1 && last.volume)
                            *out++ = last;

                        if (sent_1 == N)
                            return;

                        last.price = before_first[idx]->price;
                        last.volume = before_first[idx]->volume;
                        ++sent_1;
                    }
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

		template <typename Order>
			void onPartiallyFilled(Order const & order, PriceVolume const & trade)
			{}

		DECLARE_ARROW(OrderQueue);

#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T>
            static void py_visit(T & class_def)
            {
                class_def
                     .def("empty",       &OrderQueue::empty)
                     .def("bestVolume",  &OrderQueue::getBestVolume)
                     .def("bestPrice",   &OrderQueue::getBestPrice)
                     .def("bestOrders",  &OrderQueue::getBestOrders)
                    ;
            }

            boost::python::list getBestOrders(int N) const 
            {
                std::vector<PriceVolume>  const &  best = getBestN(N);
                boost::python::list result;
                BOOST_FOREACH(PriceVolume const & x, best)
                    result.append(x);
                return result;
            }
#endif

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

    struct BestPriceAndVolume 
    {
        typedef PriceVolume	ValueType;

        template <typename T> 
        static ValueType getValue(T x) 
        {
            return PriceVolume(x->getBestPrice(), x->getBestVolume());
        }
    };

    template <typename Handler, typename Base>
		struct OnQueueTopChanged : Base
	{
		OnQueueTopChanged() {}

/*
		template <typename T> OnQueueTopChanged(T const & x) 
			: Base	  (boost::get<0>(x))
			, handler_(boost::get<1>(x))
		{}
*/

		DECLARE_BASE(OnQueueTopChanged);

		using Base :: value_type;

		template <typename T>
			void push(T x)
			{
				Base::push(x);
				if (!comp(x, top()))
					handler_(self());
			}

		template <typename T>
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

		template <typename Order>
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

        OnQueueTopChanged(OnQueueTopChanged const &);
	};
}

#endif