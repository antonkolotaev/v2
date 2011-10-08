#ifndef _marketsim_order_cancellable_h_included_
#define _marketsim_order_cancellable_h_included_

namespace marketsim {
namespace order     {
    /// Base class for orders that are to stored in OrdersSubmittedInVector
    /// We will store index of the order in the vector
    /// providing thus fast search and remove operations
    template <typename Base>
        struct WithCancelPosition : Base 
    {
        template <typename T> 
            WithCancelPosition(T const & x) 
                :   Base(x)
                ,   pos_(-1) 
            {}

        DECLARE_BASE(WithCancelPosition);

        void setCancelPosition(int p) { pos_ = p; }
        int  getCancelPosition() const { return pos_; }

    private:
        int     pos_;
    };

}}

#endif