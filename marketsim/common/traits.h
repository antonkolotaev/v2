#ifndef _marketsim_common_traits_h_included_
#define _marketsim_common_traits_h_included_

namespace marketsim
{
    /// -------------------- Buy/Sell side

    /// an enum representing a trade side Sell/Ask or Buy/Bid
    enum Side { Sell, Buy, Ask = Sell, Bid = Buy };

    /// class template for representing different values of enum Side
    template <Side SIDE> struct side_tag { enum { value = SIDE }; typedef side_tag type; };

    typedef side_tag<Sell>  sell_tag;
    typedef side_tag<Buy>   buy_tag;

    /// meta function returning a tag class representing the side of the order
    template <typename Order> struct order_side
    {
        // by default we rely on order_side_tag inner type in T
        // if it is not your case specialize this template
        typedef typename Order :: order_side_tag    type;
    };

    /// the side of pointers to orders is the side of the orders itself
    template <typename Order> struct order_side<Order*> : order_side<Order> {};
    template <typename Order> struct order_side<boost::intrusive_ptr<Order> > : order_side<Order> {};

    /// meta function returning tag of the opposite side for given tag: 
    /// sell_tag -> buy_tag; buy_tag -> sell_tag
    template <typename SideTag> struct opposite_side;

    template <> struct opposite_side<sell_tag> : buy_tag {};
    template <> struct opposite_side<buy_tag> : sell_tag {};

    ///---------------------  Limit/Market category

    /// Tag class for representing limit orders 
    struct limit_order_tag {};
    /// Tag class for representing market orders
    struct market_order_tag {};

    /// meta function giving the category (market or limit) of the order type
    template <typename Order> struct order_category
    {
        /// By default we rely on 'category' typename within the order class
        typedef typename Order :: category type;
    };

    /// category of a pointer to order is the category of the order itself
    template <typename Order> struct order_category<Order*> : order_category<Order> {};
    template <typename Order> struct order_category<boost::intrusive_ptr<Order> > : order_category<Order> {};
}

#endif