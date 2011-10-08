#ifndef _marketsim_order_side_h_included_
#define _marketsim_order_side_h_included_

#include <numeric>
#include <marketsim/common/traits.h>
#include <marketsim/common/PriceVolume.h>

namespace marketsim {
namespace order     {
    /// Base class for orders encapsulating logic related to the side of the order
    template <Side SIDE>	
        struct side_is
        {
            /// this type name is used by order traits
            typedef side_tag<SIDE>  order_side_tag;

            static const Side OTHER_SIDE = SIDE == Buy ? Sell : Buy;

            /// Calculates P&L of a trade to an agent issued this order
            /// so it is negative for purchase and positive for sell
            /// \param x result of the trade
            static Price calculatePnL(PriceVolume const & x)
            {
                return SIDE == Buy ? -x.PnL_Raw() : +x.PnL_Raw();
            }

            /// Calculates change in amount of assets held by an agent after a trade
            /// so it is positive for purchase and negative for sell
            /// \param v amount of assets traded
            static Volume calculateQuantity(Volume v)
            {
                return SIDE == Buy ? +v : -v;
            }

            /// \return a price of the worst order in sense of price
            /// so it is an buy order with 0 price or a sell order with infinite price
            static Price worstPrice() 
            {
                return SIDE == Buy ? 0 : std::numeric_limits<Price>::max();
            }
        };
}}

namespace marketsim
{
#ifdef MARKETSIM_BOOST_PYTHON
    template <typename T> std::string py_name() // TO BE MOVED TO marketsim/python
    {
        return T::py_name();
    }

    template <> std::string py_name<sell_tag>() { return "Sell"; }
    template <> std::string py_name<buy_tag>() { return "Buy"; }
#endif
}

#endif