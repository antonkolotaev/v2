#ifndef _marketsim_history_h_included_
#define _marketsim_history_h_included_

#include <queue>
#include <boost/optional.hpp>
#include <marketsim/scheduler.h>

#include <fstream>

namespace marketsim {
namespace history {

	template <class FieldType>
		struct InDeque 
		{
			typedef std::pair<Time,FieldType>       HistoryPiece;
			typedef std::deque<HistoryPiece>        HistoryStorage;

			InDeque() {}
			template <class T> 
				InDeque(T const & x) {}

			HistoryStorage const & getHistory() 
			{
				return history_;
			}

		protected:
			void write(Time t, FieldType const & x)
			{
				history_.push_back(HistoryPiece(t,x));
			}
			

		private:
			HistoryStorage  history_;
		};

		struct InFile 
		{
			InFile(const char * filename) : out_(filename) {}

		protected:
			template <class FieldType>
				void write(Time t, FieldType const & x)
			{
				out_ << t << "\t" << x << "\n";
			}			

		private:
			std::ofstream	out_;
		};


		template <class FieldTag, class Base = InDeque<typename FieldTag::ValueType> >
			struct Collector : Base
        {
            typedef FieldTag                        Tag;
            typedef typename FieldTag :: ValueType  FieldType;
            typedef std::pair<Time,FieldType>       HistoryPiece;
            typedef std::deque<HistoryPiece>        HistoryStorage;

			Collector() {}

			template <class T>
				Collector(T const & x) : Base(x), recording_(true), lastT_(-1.) {}

            void recordHistory(bool bRecord = true) 
            {
                recording_ = bRecord;
            }

			template <class T>
				void operator () (T x)
			{
                if (recording_)
                {
                    Time t = scheduler().currentTime();
                    FieldType p = FieldTag::getValue(x);

					if (lastT_ != t)
					{
						if (last_)
							write(lastT_, *last_);
					}
					last_ = p;
					lastT_ = t;
                }
			}

			void flush() 
			{
				if (last_)
				{
					write(lastT_, *last_);
					last_.reset(); lastT_ = -1;
				}
			}

			~Collector() 
			{
				flush();
			}

        private:
			Time						lastT_;
			boost::optional<FieldType>	last_;
            bool						recording_;
        };


	// History encapsulates:
	//  - logic of recording: on/off
	//  - logic of filtering out values corresponding to the same time
	//  - (to be done) logic of filtering by time
	// 
	// So we may conclude that it should be parametrized by "OutputIterator"<Time, FieldType>


}}

#endif