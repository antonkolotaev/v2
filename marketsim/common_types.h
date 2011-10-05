#ifndef _marketsim_common_types_h_included
#define _marketsim_common_types_h_included

#include <boost/intrusive_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <ostream>

#pragma warning ( disable : 4355 )

#define DECLARE_ARROW(X) \
    X *         self ()      { return  static_cast<X*>(this); } \
    X const *   self () const{ return  static_cast<X const *>(this); } \
    X *         operator -> ()      { return  static_cast<X*>(this); } \
    X const *   operator -> () const{ return  static_cast<X const *>(this); } \
    X &         operator * ()       { return  static_cast<X&>(*this); } \
    X const &   operator * () const { return  static_cast<X const&>(*this); } 
 
#define DECLARE_BASE(X) typedef X base; typedef X type;

namespace marketsim 
{
    enum Side { Sell, Buy, Ask = Sell, Bid = Buy };

    template <Side side> struct side_tag { enum { value = side }; };

    typedef side_tag<Sell>  sell_tag;
    typedef side_tag<Buy>   buy_tag;

    struct limit_order_tag {};
    struct market_order_tag {};

    template <typename T> struct order_category
    {
        typedef typename T :: category type;
    };

    template <typename T> struct order_category<T*> : order_category<T> {};

    template <typename T> struct order_side
    {
        // by default we rely on order_side_tag inner type in T
        // if it is not your case specialize this template
        typedef typename T :: order_side_tag    type;
    };

    template <typename T> struct order_side<T*> : order_side<T> {};
    template <typename T> struct order_side<boost::intrusive_ptr<T> > : order_side<T> {};

    template <typename SideTag> struct opposite_side;

    template <Side S> struct opposite_side<side_tag<S> > 
    {
        typedef side_tag<S == Buy ? Sell : Buy>     type;
    }; 

    template <typename T>
    struct ordered_by_price 
    {
        typedef typename T :: OrderedByPrice type;
    };

	typedef int Price;
	typedef int Volume;
	typedef double Time;

    typedef double PriceF;
    typedef double VolumeF;


    template <typename T> struct ordered_by_price<boost::intrusive_ptr<T> > 
    {
        struct type 
        {
			bool operator () (boost::intrusive_ptr<T> const & lhs, boost::intrusive_ptr<T> const & rhs) const 
			{
				typedef typename ordered_by_price<T>::type  X;
				return X()(*lhs, *rhs);
			}
			bool operator () (boost::intrusive_ptr<T> const & lhs, Price rhs) const 
			{
				typedef typename ordered_by_price<T>::type  X;
				return X()(*lhs, rhs);
			}
        };
    };

	struct Empty {};

    enum Dummy { dummy };

    template <typename T>
    std::string toStr(T const &x) 
    {
        return (std::stringstream() << x).str();
    }


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

    inline PriceVolume pv(Price p, Volume v) { return PriceVolume(p,v); }
}

namespace std 
{
    template <typename T>
        struct less<boost::intrusive_ptr<T> >
        {
            bool operator () (boost::intrusive_ptr<T> const & lhs, boost::intrusive_ptr<T> const & rhs)
            {
                return *lhs < *rhs;
            }
        };
}

#endif