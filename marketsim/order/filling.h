#ifndef _marketsim_order_filling_h_included_
#define _marketsim_order_filling_h_included_

#include <vector>
#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

namespace marketsim
{
    struct ExecutionHistoryPiece : PriceVolume
    {
        Time timestamp;

        ExecutionHistoryPiece(PriceVolume const & pv)
            :   PriceVolume(pv)
            ,   timestamp(Scheduler::currentTime())
        {}
        ExecutionHistoryPiece(Time t, PriceVolume const & pv)
            :   PriceVolume(pv)
            ,   timestamp(t)
        {}
    };
    typedef std::deque<PriceVolume> ExecutionHistoryStorage;   

    template <typename Base>
        struct ExecutionHistory : Base
    {
        template <typename T>
            ExecutionHistory(T const & x) : Base(x), recording_(true) {}

        typedef ExecutionHistory base;   // for derived typenamees

        template <typename OtherOrder>
        void onMatched(PriceVolume const & pv, OtherOrder const &o)
        {
            Base::onMatched(pv,o);
            if (recording_)
                history_.push_back(pv);
        }

        ExecutionHistoryStorage const & getExecutionHistory() const { return history_; }

        void recordFilling(bool bRecord = true) { recording_ = bRecord; }

    private:
        bool                        recording_;
        ExecutionHistoryStorage     history_;
    };

}


#endif