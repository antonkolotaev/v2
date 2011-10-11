#ifndef _marketsim_order_queue_for_best_n_h_included_
#define _marketsim_order_queue_for_best_n_h_included_

namespace marketsim {
namespace order_queue {

    /// Compares indices in an array using a comparer for array value types
    template <
        typename Comparer,              // Underlying comparer for element type
        typename RandomAccessIterator   // A pointer to the array base
    >
	    struct IdxComparerT
	{
		IdxComparerT(Comparer const & comp, RandomAccessIterator ptr) 
            :   comp(comp)
            ,   ptr(ptr) 
        {}

		bool operator () (size_t lhs, size_t rhs)  const
		{
			return comp(ptr[lhs], ptr[rhs]);
		}

	private:
		Comparer     const & comp;
		RandomAccessIterator ptr; 
	};

    /// Enumerates best N priority queue elements in order given by Comparer
	template <
        typename PriorityQueue,     // a random access container with a heap
        typename Comparer,          // comparer for container elements
        typename OutputIterator     // output iterator for (Price,Volume) of the order
    >
		void getBestN(PriorityQueue const & c, Comparer const & comp, int N, OutputIterator out)  
	{
        // current (Price,Volume)
		PriceVolume		last(-1,-1);
        // number of orders sent + 1
		int  			sent_1 = 0;

        if (c.empty())
            return;

        typedef typename PriorityQueue :: value_type const *        const_iterator;
        typedef typename IdxComparerT<Comparer, const_iterator>                         IdxComparer;
        typedef typename std::priority_queue<size_t, std::vector<size_t>, IdxComparer>  IndicesQueue;

        // it is convenient to index a heap from 1 but not from 0
		const_iterator before_first = &c[0] - 1;
        // size of the queue + 1
		size_t size_1 = c.size() + 1;

        // comparer for indices
		IdxComparer     idx_comparer(comp, before_first);
        // priority queue of "gray" set of our BFS
		IndicesQueue	indices(idx_comparer);
		
		indices.push(1);

        // if there are not processed elements
		while (!indices.empty())
		{
            // choose the best one
			size_t idx = indices.top();

            // if it has another price than previous element
			if (last.price != before_first[idx]->getPrice())
			{
                // and is meaningful
                if (!before_first[idx]->cancelled())
                {
                    // send the previous one to the output
                    if (sent_1 && last.volume)
                        *out++ = last;

                    // if N orders with different price were processed, stop
                    if (sent_1 == N)
                        return;

                    // store current order price and volume 
                    last.price = before_first[idx]->getPrice();
                    last.volume = before_first[idx]->getVolume();
                    ++sent_1;
                }
			}
			else    // if price is same, just add volume
				last.volume += before_first[idx]->getVolume();

            // proceed to next order
			indices.pop();

            // if the order has first child
			if (2*idx < size_1)
			{
                // enqueue it
				indices.push(2*idx);
                // if the order has second child 
				if (2*idx+1 < size_1)
                    // enqueue it too
					indices.push(2*idx+1);
			}
		}
        // at the end, send info about the last order to the output
		if (sent_1 && last.volume)
			*out++ = last;
	}


}}

#endif