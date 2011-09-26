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
		template <class T>
		struct IGenerator : RefCounted<IGenerator<T> >
		{
			virtual T operator () () = 0;
			virtual ~IGenerator() {}
			void on_released() { delete this; }
		};

		template <class T>
			struct Generator : boost::intrusive_ptr<IGenerator<T> >
		{
			typedef boost::intrusive_ptr<IGenerator<T> > Base;

			Generator() : Base() {}

			template <class T> Generator(T x) : Base(x) {}

			T operator () () { return Base::operator *() (); }
		};

		template <int> struct IntRngHolderT
		{
			static boost::mt19937  g_int_rnd;
		};

		template <int N> boost::mt19937 IntRngHolderT<N>::g_int_rnd;

		template <class Interface = Empty>
			struct uniform_01 : boost::uniform_01<>, Interface
        {
            typedef boost::uniform_01<> base;

			result_type operator () () { return base::operator() (IntRngHolderT<0>::g_int_rnd); }
        };

		template <class Interface = Empty>
			struct uniform_smallint : boost::uniform_smallint<>, Interface
        {
            typedef boost::uniform_smallint<> base;

            explicit uniform_smallint(int min_arg = 0, int max_arg = 9)
                : base(min_arg,max_arg)
            {}

			result_type operator () () { return base::operator ()(IntRngHolderT<0>::g_int_rnd); }
        };

        template <class T = double, class Interface = Empty>
			struct constant : Interface
        {
            explicit constant(T x) : x(x) {}

            T operator () () { return x; }

        private:
            T   x;
        };

		template <int> struct RngHolder 
		{
			static uniform_01<>	g_RNG;
		};

		template <int N> uniform_01<> RngHolder<N>::g_RNG;

		template <class Interface = Empty>
			struct exponential : boost::exponential_distribution<>, Interface
        {
            typedef boost::exponential_distribution<> base;

            explicit exponential(result_type lambda_arg = result_type(1))
                : base(lambda_arg) {}

			result_type operator () () { return base::operator ()(RngHolder<0>::g_RNG); }
        };

		template <class Interface = Empty>
			struct normal : boost::normal_distribution<>, Interface
        {
            typedef boost::normal_distribution<> base;

            explicit normal(const result_type& mean_arg = result_type(0),
                const result_type& sigma_arg = result_type(1))
                : base(mean_arg, sigma_arg) {}

			result_type operator () () { return base::operator ()(RngHolder<0>::g_RNG); }
        };

		template <class Interface = Empty>
			struct lognormal : boost::lognormal_distribution<>, Interface
        {
            typedef boost::lognormal_distribution<> base;

            explicit lognormal(result_type mean_arg = result_type(1),
                result_type sigma_arg = result_type(1))
                : base(mean_arg, sigma_arg) {}

			result_type operator () () { return base::operator ()(RngHolder<0>::g_RNG); }
        };

		template <class Interface = Empty>
			struct gamma : boost::gamma_distribution<>, Interface
        {
            typedef boost::gamma_distribution<> base;

            explicit gamma(const result_type& alpha_arg = result_type(1)) : base(alpha_arg) {}

			result_type operator () () { return base::operator ()(RngHolder<0>::g_RNG); }
        };

		template <class Interface = Empty>
			struct uniform_real : boost::uniform_real<>, Interface
        {
            typedef boost::uniform_real<> base;

            uniform_real(double min_arg = 0,
                double max_arg = 1) 
                :  base(min_arg, max_arg) {}

			result_type operator () () { return base::operator ()(RngHolder<0>::g_RNG); }
        };
    }


}

#endif