#ifndef _marketsim_order_execution_history_h_included_
#define _marketsim_order_execution_history_h_included_

#include <vector>
#include <marketsim/common_types.h>
#include <marketsim/scheduler.h>

#include <marketsim/history.h>

namespace marketsim
{
    /// We may want to collect history of trades for a given order
    /// For the moment we save (time, (Price,Volume)) for each trade
    typedef history::TimeStamped<PriceVolume>   ExecutionHistoryPiece;
    typedef history::TimeSeries<PriceVolume>    ExecutionHistoryStorage;   

    /// Base class for orders that want to collect their trade history,
    /// i.e. history of matching with other orders
    /// We will save the history in a std::deque
    template <typename Base>
        struct ExecutionHistory : Base
    {
        template <typename T>
            ExecutionHistory(T const & x) 
                :   Base        (x)
                ,   recording_  (true) 
            {}

        DECLARE_BASE(ExecutionHistory);

        template <typename OtherOrder>
            void onMatched(PriceVolume const & pv, OtherOrder const &o)
        {
            Base::onMatched(pv,o);
            
            if (recording_)
                history_.push_back(
                    ExecutionHistoryPiece(
                        Scheduler::currentTime(), 
                        pv));
        }

        ExecutionHistoryStorage const & getExecutionHistory() const { return history_; }

        /// stops or resumes recording the history
        /// \param bRecord true iff the recording is to be done
        void recordFilling(bool bRecord = true) { recording_ = bRecord; }

    private:
        bool                        recording_;
        ExecutionHistoryStorage     history_;
    };

    // maybe it has sense to extract class order::OnMatched and the rest of the class move to history namespace

}


#endif