#ifndef _marketsim_order_queue_best_price_and_volume_h_included_
#define _marketsim_order_queue_best_price_and_volume_h_included_

namespace marketsim {
namespace order_queue {

    /// A trait class used to extract price and volume of the best order from a queue
    struct BestPriceAndVolume 
    {
        typedef PriceVolume	ValueType;

        template <typename T> 
            static ValueType getValue(T x) 
        {
            return PriceVolume(x->getBestPrice(), x->getBestVolume());
        }
    };

#ifdef MARKETSIM_BOOST_PYTHON
    /// A trait class used to extract price and volume of the best order from a queue given by a Python class
    struct py_BestPriceAndVolume
    {
        typedef PriceVolume	ValueType;

        static ValueType getValue(boost::python::object x) 
        {
            return PriceVolume(
                boost::python::extract<Price>(x.attr("bestPrice")()), 
                boost::python::extract<Volume>(x.attr("bestVolume")()));
        }

        static std::string py_name() 
        {
            return "BestPriceAndVolume";
        }
    };

#endif

}}

#endif