#ifndef _marketsim_history_in_deque_h_included_
#define _marketsim_history_in_deque_h_included_

#include <marketsim/history/time_series.h>

namespace marketsim {
namespace history {

    /// Aggregates history in std::deque
    /// \param FieldType type of values to be stored
	template <typename FieldType>
		struct InDeque 
		{
			typedef TimeStamped<FieldType>        HistoryPiece;
			typedef TimeSeries<FieldType>         HistoryStorage;

			InDeque() {}

			template <typename T> 
				InDeque(T const & x) 
            {}

            //to be replaced by TimeSeries<T>& operator << (TimeSeries<T>&, TimeStamped<T> const &);
            void write(Time t, FieldType const & x)
			{
				history_.push_back(HistoryPiece(t,x));
			}

			HistoryStorage const & getHistory() const
			{
				return history_;
			}

#ifdef MARKETSIM_BOOST_PYTHON
            template <typename T>
                static void py_visit(T & c)
                {
                    c.def_readonly("history", &InDeque::history_);
                }
#endif

		private:
			HistoryStorage  history_;
		};

}}

#endif