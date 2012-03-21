#ifndef _marketsim_history_collector_h_included_
#define _marketsim_history_collector_h_included_

#include <boost/optional.hpp>
#include <marketsim/history/in_deque.h>

namespace marketsim {
namespace history   {

    /// Records value history of some field
    /// \param FieldTag - a trait class telling type of a value to be stored and how it can be extracted
    /// \param Base -- in fact it is a storage for history
	template <
        typename FieldTag, 
        typename Base = InDeque<typename FieldTag::ValueType> 
    >
		struct Collector : Base
    {
        typedef FieldTag                        Tag;
        typedef typename FieldTag :: ValueType  FieldType;

		Collector() {}

		template <typename T>
			Collector(T const & x) : Base(x), recording_(true), lastT_(-1.) {}

        /// stops/resumes history recording
        void recordHistory(bool bRecord = true) 
        {
            recording_ = bRecord;
        }

        /// updates history with a new value
        /// if time hasn't changed since the last update
        /// it just updates the last value
		template <typename T>
			void operator () (T x)
		{
            if (recording_)
            {
                Time t = Scheduler::currentTime();
                FieldType p = FieldTag::getValue(x);

				if (lastT_ != t)
				{
					if (last_)
                        this->write(lastT_, *last_);  // storage_ << TimeStamped(lastT_, *last_)
				}
				last_ = p;
				lastT_ = t;
            }
		}

        /// Pushes the last value to the storage
		void flush() 
		{
			if (last_)
			{
				this->write(lastT_, *last_);
				last_.reset(); lastT_ = -1;
			}
		}

#ifdef MARKETSIM_BOOST_PYTHON
        template <class T>
            static void py_visit(T & c)
            {
                c.def("__call__", &Collector::operator ()<object>);
                c.def_readwrite("recording", &Collector::recording_);                    
            }
#endif

		~Collector() 
		{
			flush();
		}

    private:
		Time						lastT_;
		boost::optional<FieldType>	last_;
        bool						recording_;
    };


}}

#endif
