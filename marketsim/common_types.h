#ifndef _marketsim_common_types_h_included
#define _marketsim_common_types_h_included

#include <boost/intrusive_ptr.hpp>

#pragma warning ( disable : 4355 )

#define DECLARE_ARROW(X) \
    X *         self ()      { return  static_cast<X*>(this); } \
    X const *   self () const{ return  static_cast<X const *>(this); } \
    X *         operator -> ()      { return  static_cast<X*>(this); } \
    X const *   operator -> () const{ return  static_cast<X const *>(this); } \
    X &         operator * ()       { return  static_cast<X&>(*this); } \
    X const &   operator * () const { return  static_cast<X const&>(*this); } 

namespace marketsim 
{
    enum Side { Sell, Buy, Ask = Sell, Bid = Buy };

    template <Side side> struct side_tag { enum { value = side }; };

    typedef side_tag<Sell>  sell_tag;
    typedef side_tag<Buy>   buy_tag;

    struct limit_order_tag {};
    struct market_order_tag {};

    template <class T> struct order_category
    {
        typedef typename T :: category type;
    };

    template <class T> struct order_category<T*> : order_category<T> {};

    template <class T> struct order_side
    {
        // by default we rely on order_side_tag inner type in T
        // if it is not your case specialize this template
        typedef typename T :: order_side_tag    type;
    };

    template <class T> struct order_side<T*> : order_side<T> {};

    template <class SideTag> struct opposite_side;

    template <Side S> struct opposite_side<side_tag<S> > 
    {
        typedef side_tag<S == Buy ? Sell : Buy>     type;
    };

    template <class T>
    struct ordered_by_price 
    {
        typedef typename T :: OrderedByPrice type;
    };

	typedef int Price;
	typedef int Volume;
	typedef double Time;

    template <class T> struct ordered_by_price<boost::intrusive_ptr<T> > 
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

	struct Dummy {};

	__declspec(selectany)  Dummy dummy;


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
    };

    inline PriceVolume pv(Price p, Volume v) { return PriceVolume(p,v); }
}

namespace std 
{
    template <class T>
        struct less<boost::intrusive_ptr<T> >
        {
            bool operator () (boost::intrusive_ptr<T> const & lhs, boost::intrusive_ptr<T> const & rhs)
            {
                return *lhs < *rhs;
            }
        };
}

#endif