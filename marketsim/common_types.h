#ifndef _marketsim_common_types_h_included
#define _marketsim_common_types_h_included

#include <boost/intrusive_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <sstream>

namespace marketsim
{
    /// converts an object to std::string by calling its operator << 
    /// analogous to boost::lexical_cast<std::string>
    template <typename T>
        std::string toStr(T const &x) 
    {
        return (std::stringstream() << x).str();
    }

    /// ------------------------------- miscellaneous
    /// an empty base class
	struct Empty {};

    /// a dummy value 
    /// we use it in order to pass something to a function requiring a parameter
    enum Dummy { dummy };
}

#include <marketsim/common/macros.h>
#include <marketsim/common/PriceVolume.h>
#include <marketsim/common/traits.h>

#pragma warning ( disable : 4355 )

#endif