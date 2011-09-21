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
        struct OrderQueue : private std::priority_queue<Order, std::vector<Order>, typename ordered_by_price<Order>::type>
    {
        typedef 
            std::priority_queue<Order, std::vector<Order>, typename ordered_by_price<Order>::type>
            base;

        using base::reference;
        using base::push;
        using base::top;

        void pop() // we suppose that the order on top is valid
        { 
            base::pop(); 
            make_valid();
        }

        bool empty()
        {
            make_valid();
            return base::empty();
        }

        struct Cancalled {
            bool operator () (Order const & q) const {
                return q->cancelled();
            }
        };

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
}

#endif