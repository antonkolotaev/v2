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

#ifdef MARKETSIM_BOOST_PYTHON
            template <typename T>
                static void py_visit(T & c)
                {
                    c.def_readonly("history", &InDeque::history_);
                }
#endif

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

        template <class FieldTag>   
            struct CollectInDeque : Collector<FieldTag>
            {
                typedef Collector<FieldTag>         Base;
                typedef typename Base::FieldType    FieldType;

                template <class T> 
                    CollectInDeque(T const & x) : Base(x)
                    {}

                CollectInDeque() {}

                TimeSerie<FieldType> const & getHistory() 
                {
                    Base::flush();
                    return Base::getHistory();
                }

#ifdef MARKETSIM_BOOST_PYTHON 

                static std::string py_name() 
                {
                    return "HistoryInDeque_" + marketsim::py_name<FieldTag>();
                }

                static void py_register()
                {
                    using namespace boost::python;
                    ::py_register<HistoryStorage>();
                    class_<CollectInDeque, boost::noncopyable> c(py_name().c_str());
                    c.def("getHistory", &CollectInDeque::getHistory, return_internal_reference<>());
                    Base::py_visit(c);
                }
#endif
            };


	// History encapsulates:
	//  - logic of recording: on/off
	//  - logic of filtering out values corresponding to the same time
	//  - (to be done) logic of filtering by time
	// 
	// So we may conclude that it should be parametrized by "OutputIterator"<Time, FieldType>


}}

#endif