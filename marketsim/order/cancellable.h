#ifndef _marketsim_order_cancellable_h_included_
#define _marketsim_order_cancellable_h_included_

namespace marketsim 
{
    template <typename Base>
        struct WithCancelPosition : Base 
    {
        template <typename T> WithCancelPosition(T const & x) : Base(x), pos_(-1) {}

        typedef WithCancelPosition  base; // for derived typenamees

        void setCancelPosition(int p) { pos_ = p; }
        int  getCancelPosition() const { return pos_; }

    private:
        int     pos_;
    };

}

#endif