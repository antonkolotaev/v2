#ifndef _marketsim_rng_h_included_
#define _marketsim_rng_h_included_

#include <boost/random.hpp>
#include <boost/intrusive_ptr.hpp>

#include <marketsim/ref_counted.h>
#include <marketsim/common_types.h>

namespace marketsim
{
    namespace rng 
    {
        /// interface for random number generators managed by boost::intrusive_ptr
		template <typename T>
		struct IGenerator : RefCounted<IGenerator<T> >
		{
			virtual T operator () () = 0;
			virtual ~IGenerator() {}
			void on_released() { delete this; }
		};

		template <typename T>
			struct Generator : boost::intrusive_ptr<IGenerator<T> >
		{
			typedef boost::intrusive_ptr<IGenerator<T> > Base;

			Generator() : Base() {}

			template <typename U> Generator(U x) : Base(x) {}

			T operator () () { return Base::operator *() (); }
		};

		template <int> struct IntRngHolderT
		{
			static boost::mt19937  g_int_rnd;
		};

		template <int N> boost::mt19937 IntRngHolderT<N>::g_int_rnd;

		template <typename Interface = Empty>
			struct uniform_01 : boost::uniform_01<>, Interface
        {
            typedef boost::uniform_01<> base;

			result_type operator () () { return base::operator() (IntRngHolderT<0>::g_int_rnd); }

#ifdef MARKETSIM_BOOST_PYTHON
            static void py_register()
            {
                using namespace boost::python;

                class_<uniform_01>("uniform_01")
                    .def("__call__", &uniform_01::operator ())
                    ;
            }
#endif
        };

		template <typename Interface = Empty>
			struct uniform_smallint : boost::uniform_smallint<>, Interface
        {
            typedef boost::uniform_smallint<> base;

            explicit uniform_smallint(int min_arg = 0, int max_arg = 9)
                : base(min_arg,max_arg)
            {}

			result_type operator () () { return base::operator ()(IntRngHolderT<0>::g_int_rnd); }

#ifdef MARKETSIM_BOOST_PYTHON
            static void py_register()
            {
                using namespace boost::python;

                class_<uniform_smallint>("uniform_smallint", boost::python::init<int,int>())
                    .def("__call__", &uniform_smallint::operator ())
                    ;
            }
#endif
        };

        template <typename T = double, typename Interface = Empty>
			struct constant : Interface
        {
            explicit constant(T x) : x(x) {}

            T operator () () { return x; }

#ifdef MARKETSIM_BOOST_PYTHON
            static void py_register()
            {
                using namespace boost::python;

                class_<constant>("constant", boost::python::init<T>())
                    .def("__call__", &constant::operator ())
                    ;
            }
#endif

        private:
            T   x;
        };

		template <int> struct RngHolder 
		{
			static uniform_01<>	g_RNG;
		};

		template <int N> uniform_01<> RngHolder<N>::g_RNG;

		template <typename T = double, typename Interface = Empty>
			struct exponential : boost::exponential_distribution<T>, Interface
        {
            typedef boost::exponential_distribution<T> base;

            explicit exponential(T lambda_arg = T(1))
                : base(lambda_arg) {}

			T operator () () { return base::operator ()(RngHolder<0>::g_RNG); }

#ifdef MARKETSIM_BOOST_PYTHON
            static void py_register()
            {
                using namespace boost::python;

                class_<exponential>("exponential", boost::python::init<result_type>())
                    .def("__call__", &exponential::operator ())
                    ;
            }
#endif
        };

		template <typename T = double, typename Interface = Empty>
			struct normal : boost::normal_distribution<T>, Interface
        {
            typedef boost::normal_distribution<T> base;

            explicit normal(const T& mean_arg = T(0),
                const T& sigma_arg = T(1))
                : base(mean_arg, sigma_arg) {}

			T operator () () { return base::operator ()(RngHolder<0>::g_RNG); }

#ifdef MARKETSIM_BOOST_PYTHON
            static void py_register()
            {
                using namespace boost::python;

                class_<normal>("normal", boost::python::init<result_type, result_type>())
                    .def("__call__", &normal::operator ())
                    ;
            }
#endif
        };

		template <typename T = double, typename Interface = Empty>
			struct lognormal : boost::lognormal_distribution<T>, Interface
        {
            typedef boost::lognormal_distribution<T> base;

            explicit lognormal(T mean_arg = T(1),
                T sigma_arg = T(1))
                : base(mean_arg, sigma_arg) {}

			T operator () () { return base::operator ()(RngHolder<0>::g_RNG); }

#ifdef MARKETSIM_BOOST_PYTHON
            static void py_register()
            {
                using namespace boost::python;

                class_<lognormal>("lognormal", boost::python::init<double, double>())
                    .def("__call__", &lognormal::operator ())
                    ;
            }
#endif

        };

		template <typename T = double, typename Interface = Empty>
			struct gamma : boost::gamma_distribution<T>, Interface
        {
            typedef boost::gamma_distribution<T> base;

            explicit gamma(const T& alpha_arg = T(1)) : base(alpha_arg) {}

			T operator () () { return base::operator ()(RngHolder<0>::g_RNG); }

#ifdef MARKETSIM_BOOST_PYTHON
            static void py_register()
            {
                using namespace boost::python;

                class_<gamma>("gamma", boost::python::init<double>())
                    .def("__call__", &gamma::operator ())
                    ;
            }
#endif

        };

		template <typename T = double, typename Interface = Empty>
			struct uniform_real : boost::uniform_real<T>, Interface
        {
            typedef boost::uniform_real<T> base;

            uniform_real(double min_arg = 0,
                double max_arg = 1) 
                :  base(min_arg, max_arg) {}

			T operator () () { return base::operator ()(RngHolder<0>::g_RNG); }

#ifdef MARKETSIM_BOOST_PYTHON
            static void py_register()
            {
                using namespace boost::python;

                class_<uniform_real>("uniform_real", boost::python::init<result_type,result_type>())
                    .def("__call__", &uniform_real::operator ())
                    ;
            }
#endif
        };
    }


}

#endif
