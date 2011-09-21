#ifndef _marketsim_rng_h_included_
#define _marketsim_rng_h_included_

#include <boost/random.hpp>

namespace marketsim
{
    namespace rng 
    {
        __declspec(selectany) boost::mt19937  g_int_rnd;

        struct uniform_01 : boost::uniform_01<>
        {
            typedef boost::uniform_01<> base;

            result_type operator () () { return base::operator() (g_int_rnd); }
        };

        struct uniform_smallint : boost::uniform_smallint<>
        {
            typedef boost::uniform_smallint<> base;

            explicit uniform_smallint(int min_arg = 0, int max_arg = 9)
                : base(min_arg,max_arg)
            {}

            result_type operator () () { return base::operator ()(g_int_rnd); }
        };

        template <class T = double>
            struct constant
        {
            explicit constant(T x) : x(x) {}

            T operator () () const { return x; }

        private:
            T   x;
        };

        __declspec(selectany) uniform_01     g_RNG;

        struct exponential : boost::exponential_distribution<>
        {
            typedef boost::exponential_distribution<> base;

            explicit exponential(result_type lambda_arg = result_type(1))
                : base(lambda_arg) {}

            result_type operator () () { return base::operator ()(g_RNG); }
        };

        struct normal : boost::normal_distribution<>
        {
            typedef boost::normal_distribution<> base;

            explicit normal(const result_type& mean_arg = result_type(0),
                const result_type& sigma_arg = result_type(1))
                : base(mean_arg, sigma_arg) {}

            result_type operator () () { return base::operator ()(g_RNG); }
        };

        struct lognormal : boost::lognormal_distribution<>
        {
            typedef boost::lognormal_distribution<> base;

            explicit lognormal(result_type mean_arg = result_type(1),
                result_type sigma_arg = result_type(1))
                : base(mean_arg, sigma_arg) {}

            result_type operator () () { return base::operator ()(g_RNG); }
        };

        struct gamma : boost::gamma_distribution<>
        {
            typedef boost::gamma_distribution<> base;

            explicit gamma(const result_type& alpha_arg = result_type(1)) : base(alpha_arg) {}

            result_type operator () () { return base::operator ()(g_RNG); }
        };

        struct uniform_real : boost::uniform_real<>
        {
            typedef boost::uniform_real<> base;

            uniform_real(double min_arg = 0,
                double max_arg = 1) 
                :  base(min_arg, max_arg) {}

            result_type operator () () { return base::operator ()(g_RNG); }
        };
    }


}

#endif