#ifndef _marketsim_common_price_volume_h_included_
#define _marketsim_common_price_volume_h_included_

namespace marketsim
{
    /// -------------------- measure units
    /// we use typedefs in order to improve readability
    /// and also in order to improve type checking we might have used dedicated classes (like Boost.Units)
    typedef int Price;      // in order to assure comparing we use integral types for Price and Volume
    typedef int Volume;

    typedef double PriceF;  // floating point counterparts to Price and Volume
    typedef double VolumeF; // used as return values of random number generators

    /// -------------------------------- PriceVolume

    /// A pair (Price, Volume) 
    /// we introduce it due to its very frequent usage: as parameter of a limit order; as characteristic of a trade etc.
    /// In function of its meaning, price and volume may be signed or unsigned
    struct PriceVolume 
    {
        Price   price;
        Volume  volume;

        PriceVolume() {}
        PriceVolume(Price price, Volume volume) : price(price), volume(volume) {}

        friend bool operator == (PriceVolume const & lhs, PriceVolume const & rhs)
        {
            return lhs.price == rhs.price && lhs.volume == rhs.volume;
        }

        /// if PriceVolume represents a trade, this function returns amount of the money involved
        /// NB! attention should be paid to signs of the values
        Price PnL_Raw() const { return price * volume; }

		template <typename Stream>
		friend Stream& operator << (Stream& out, PriceVolume const & x)
			{
				out << "{ Price=" << x.price << " Volume=" << x.volume << " }";
				return out;
			}

#ifdef MARKETSIM_BOOST_PYTHON

        static std::string py_name() { return "PriceVolume"; }

        static void py_register(std::string const& name = py_name())
        {
            using namespace boost::python;
            class_<PriceVolume>(name.c_str(), init<Price,Volume>())
                .def_readwrite("price", &PriceVolume::price)
                .def_readwrite("volume", &PriceVolume::volume)
                .def("__str__",  toStr<PriceVolume>)
                .def("__repr__", toStr<PriceVolume>)
                ;
        }
#endif 
    };

    /// shortcut for the constructor of PriceVolume
    inline PriceVolume pv(Price p, Volume v) { return PriceVolume(p,v); }

}

#endif