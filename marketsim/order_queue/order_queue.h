#ifndef _marketsim_order_queue_order_queue_h_included_
#define _marketsim_order_queue_order_queue_h_included_

#include <queue>
#include <algorithm>
#include <boost/mpl/if.hpp>
#include <boost/foreach.hpp>

#include <boost/optional.hpp>

#include <marketsim/common_types.h>
#include <marketsim/order/ordered_by_price.h>

#include <marketsim/order_queue/for_keys_better_than.h>
#include <marketsim/order_queue/for_best_n.h>

namespace marketsim {
namespace order_queue
{
    /// Base class for order queue
    /// Maintains a heap (std::priority_queue) of orders sorted by price
    /// Since non-best order removal takes O(nlogn), we do it lazily
    // requires: 
    //      Order is a pointer-like type
    //      order->cancelled() returns true iff the order is useless and thus may be removed
    template <
        typename Order,     /// Order storage type (usually it a smart pointer to order type)
        typename Derived_t = boost::mpl::na     /// the most derived class if any
    >
        struct OrderQueue 
    {
    private:
        OrderQueue(OrderQueue const &);
    protected:
        typedef std::vector<Order>  _Container;
        typedef typename order::ordered_by_price<Order>::type _Pr;
        typedef _Container container_type;

        _Container c;	// the underlying container
        _Pr comp;	// the comparator functor

    public:

        typedef typename _Container::value_type value_type;
        typedef typename _Container::size_type size_type;
        typedef typename _Container::reference reference;
        typedef typename _Container::const_reference const_reference;

        /// the most derived type 
        typedef typename boost::mpl::if_na<Derived_t, OrderQueue>::type    Derived;

        /// the order comparer type 
        typedef typename order::ordered_by_price<Order>::type comparer_type;

		OrderQueue() {}

		OrderQueue(Dummy) {}

        bool empty() const
        {	// test if queue is empty
            return (c.empty());
        }

        size_type size() const
        {	// return length of queue
            return (c.size());
        }

        const_reference top() const
        {	// return highest-priority element
            return (c.front());
        }

        reference top()
        {	// return mutable highest-priority element (retained)
            return (c.front());
        }

    private:

        void push_impl(const value_type& _Pred)
        {	// insert value in priority order
            c.push_back(_Pred);
            push_heap(c.begin(), c.end(), comp);
        }

        void pop_impl()
        {	// erase highest-priority element
            pop_heap(c.begin(), c.end(), comp);
            c.pop_back();
        }

    public:

        /// pops the best order from the queue
        void pop() // we suppose that the order on top is valid
        { 
            pop_impl(); 
            make_valid();
        }

        /// pushes an order into the queue
        template <class T>
            void push(T order)
        {
            assert(!order->cancelled());
            push_impl(order);
        }

        /// functor returning true iff an order is cancelled
        struct Cancelled {
            bool operator () (Order const & q) const {
                return q->cancelled();
            }
        };

        /// reaction on order cancellation
        template <typename T>
            void onOrderCancelled(T const& x)
            {
                make_valid();
            }

        /// \return true iff the order is in the queue
        bool contains(Order order) const
        {
            return std::find(c.begin(), c.end(), order) != c.end();
        }

        /// \return orders sorted by price
        std::vector<Order>  getSorted() 
        {
            std::vector<Order> v(c.begin(), c.end());
            std::sort_heap(v.begin(), v.end(), this->comp);
            v.erase(std::remove_if(v.begin(), v.end(), Cancelled()), v.end());
            std::reverse(v.begin(), v.end());
            return v;
        }

        friend std::ostream & operator << (std::ostream & out, OrderQueue const & q)
        {
            out << "[";
            std::copy(q.c.begin(), q.c.end(), std::ostream_iterator<Order>(out, "; "));
            out << "]";
            return out;
        }


        /// a functor accumulating order volumes
		struct AccVolume 
		{
			template <typename OrderT>
				bool operator () (OrderT const & order)
			{
				result += order->getVolume();				
				return true;
			}

			AccVolume() : result(0) {}

			Volume result;
		};

        /// \return volume of orders of price better or equal to p
		Volume volumeForBetterPrices(Price p) const 
		{
			AccVolume acc_1;
			forKeysBetterThan(p, &c[0]-1, c.size()+1, comp, acc_1);	
			return acc_1.result;
		}

        /// \return volume of orders with the best price 
		Volume getBestVolume() const 
		{
			return empty() ? 0 : volumeForBetterPrices(top()->getPrice());
		}

        /// \return price of the best order if any; the worst possible price otherwise
        Price getBestPrice() const 
        {
            /// !!!! We should use a free meta function in order to access to the real order type
            typedef typename Order::element_type  E;
            return empty() ? E::worstPrice() : top()->getPrice();
        }

        /// \return a sorted vector of (Price,Volume) of orders with N best prices
		std::vector<PriceVolume>	getBestN(int N) const 
		{
			std::vector<PriceVolume>	result;
			result.reserve(N);
            marketsim::order_queue::getBestN(c, comp, N, std::back_inserter(result));
			return result;
		}
        
        /// reaction on a partial filling of an order
		template <typename OrderT>
			void onPartiallyFilled(OrderT const & order, PriceVolume const & trade)
			{
                /// just remember this trade
                lastTrade_ = trade;
            }

        /// \return volume of the last trade, 0 otherwise
        Volume lastTradedVolume() const 
        {
            return lastTrade_ ? lastTrade_.get().volume : 0;
        }

        /// \return price of the last trade, the worst price otherwise
        Price lastTradedPrice() const 
        {
            typedef typename Order::element_type  E;
            return lastTrade_ ? lastTrade_.get().price : E::worstPrice();
        }

        typedef Derived derived_t;

		DECLARE_ARROW(Derived);

#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T>
            static void py_visit(T & class_def)
            {
                class_def
                     .def("empty",       &OrderQueue::empty)
                     .def("bestVolume",  &OrderQueue::getBestVolume)
                     .def("bestPrice",   &OrderQueue::getBestPrice)
                     .def("bestOrders",  &OrderQueue::getBestOrders)
                     .def("lastTradedPrice", &OrderQueue::lastTradedPrice)
                     .def("lastTradedVolume",&OrderQueue::lastTradedVolume)
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

        /// makes the queue either empty, either with the valid top order
        void make_valid()
        {
            while (!empty())
            {
                reference t = top();
                if (t->cancelled())
                    pop_impl();
                else break;
            }
        }

        /// last trade information; TBD: extract this into a separate class
        boost::optional<PriceVolume>    lastTrade_;
    };

}}

#endif
