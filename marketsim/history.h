#ifndef _marketsim_history_h_included_
#define _marketsim_history_h_included_

#include <queue>
#include <marketsim/scheduler.h>

namespace marketsim
{
    template <class FieldTag>
        struct HistoryT 
        {
            typedef FieldTag                        Tag;
            typedef typename FieldTag :: ValueType  FieldType;
            typedef std::pair<Time,FieldType>       HistoryPiece;
            typedef std::deque<HistoryPiece>        HistoryStorage;

            HistoryT() : recording_(true) {}

            void recordHistory(bool bRecord = true) 
            {
                recording_ = bRecord;
            }

            template <class T>
                void update(T x)
                {
                    if (recording_)
                    {
                        Time t = scheduler().currentTime();
                        FieldType p = FieldTag::getValue(x);

                        if (history_.empty() || history_.back().first != t)
                            history_.push_back(PnLHistoryPiece(t, p));
                        else 
                            history_.back().second = p; // is it reasonable?
                    }
                }

            HistoryStorage const & getHistory() const 
            {
                return history_;
            }

        private:
            HistoryStorage  history_;
            bool            recording_;
        };

    struct History 
    {
        template <class Tag>
            struct apply 
            {
                typedef HistoryT<Tag>    type;
            };
    };
}

#endif