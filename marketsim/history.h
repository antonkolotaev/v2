#ifndef _marketsim_history_h_included_
#define _marketsim_history_h_included_

#include <queue>
#include <boost/optional.hpp>
#include <marketsim/scheduler.h>

#include <fstream>

namespace marketsim {
namespace history {

    template <typename T>
        struct TimeStamped : std::pair<Time, T>
        {
            TimeStamped(Time t, T const &x)
                :   std::pair<Time,T>(t,x)
            {}

            template <typename Stream>
                friend Stream& operator << (Stream &out, TimeStamped const & x)
                {
                    out << "{ t=" << x.first << " " << x.second << "}";
                    return out;
                }

#ifdef MARKETSIM_BOOST_PYTHON
            static std::string py_name() { return "TimeStamped_" + T::py_name(); }

            static void py_register(std::string const & name = py_name())
            {
                using namespace boost::python;

                ::py_register<T>();

                class_<TimeStamped>(name.c_str(), init<Time, T>())
                    .def_readonly("time",  &TimeStamped::first)
                    .def_readonly("value", &TimeStamped::second)
                    .def("__str__", &toStr<TimeStamped>)
                    .def("__repr__", &toStr<TimeStamped>)
                    ;
            }
#endif
        };

    template <typename T>
        struct TimeSerie
            :   std::deque<TimeStamped<T> >
        {
#ifdef MARKETSIM_BOOST_PYTHON
            static std::string py_name() 
            {
                return "TimeSerie_" + T::py_name();
            }

            static void py_register(std::string const &name = py_name())
            {
                using namespace boost::python;
                ::py_register<TimeStamped<T> >();

                class_<TimeSerie>(name.c_str())
                    .def("__iter__", boost::python::iterator<TimeSerie>())
                    ;
            }
#endif
        };

	template <typename FieldType>
		struct InDeque 
		{
			typedef TimeStamped<FieldType>        HistoryPiece;
			typedef TimeSerie<FieldType>          HistoryStorage;

			InDeque() {}
			template <typename T> 
				InDeque(T const & x) {}

			HistoryStorage const & getHistory() const
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
			template <typename FieldType>
				void write(Time t, FieldType const & x)
			{
				out_ << t << "\t" << x << "\n";
			}			

		private:
			std::ofstream	out_;
		};


		template <typename FieldTag, typename Base = InDeque<typename FieldTag::ValueType> >
			struct Collector : Base
        {
            typedef FieldTag                        Tag;
            typedef typename FieldTag :: ValueType  FieldType;
            typedef std::pair<Time,FieldType>       HistoryPiece;
            typedef std::deque<HistoryPiece>        HistoryStorage;

			Collector() {}

			template <typename T>
				Collector(T const & x) : Base(x), recording_(true), lastT_(-1.) {}

            void recordHistory(bool bRecord = true) 
            {
                recording_ = bRecord;
            }

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