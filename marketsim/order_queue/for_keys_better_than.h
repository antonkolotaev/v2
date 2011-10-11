#ifndef _marketsim_order_queue_for_keys_better_than_h_included_
#define _marketsim_order_queue_for_keys_better_than_h_included_

namespace marketsim {
namespace order_queue 
{
    /// Applies given function to all orders better than x with respect to Comparer
	template <
        typename ValueType,     // type of the threshold  (usually Price)
        typename RanIt,         // a pointer to "before first" element of a heap
        typename Comparer,      // comparer for heap elements
        typename Function       // function to be called while visiting heap elements
    >
		struct ForKeysBetterThan
	{
		ForKeysBetterThan(ValueType const & x, RanIt const & it, size_t N, Comparer const & p, Function & F)
			:	x(x), it(it), p(p), N(N), F(F)
		{
			Walk(1);
		}

	private:
        /// depth-first search starting from idx-th element
		void Walk(size_t Idx)
		{
			assert(Idx >= 1);
			assert(Idx < N);

            // if current element is better than the threshold
			if (!p(it[Idx], x))
			{
                // call the function with the current element
                // the function may stop visiting by returning false
				if (!F(it[Idx]))
					return;

                // if we have a child
				if (2*Idx < N)
				{
                    // visit it
					Walk(2*Idx);

                    // if we have a second child
					if (2*Idx+1 < N)
                        // visit it too
						Walk(2*Idx+1);
				}
			}
		}

	private:
		ValueType const & x;
		RanIt	  const & it;
		size_t	  const   N;
		Comparer  const & p;
		Function		& F;
	};

    /// Applies given function to all orders better than x with respect to Comparer
	template <
        typename ValueType,     // type of the threshold  (usually Price)
        typename RanIt,         // a pointer to "before first" element of a heap
        typename Comparer,      // comparer for heap elements
        typename Function       // function to be called while visiting heap elements
    >
	void forKeysBetterThan(ValueType const & x, RanIt const & it, size_t N, Comparer const & p, Function & F)
    {
        ForKeysBetterThan<ValueType, RanIt, Comparer, Function>(x,it,N,p,F);
    }


}}

#endif