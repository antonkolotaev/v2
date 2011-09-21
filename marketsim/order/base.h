#ifndef _marketsim_order_base_h_included_
#define _marketsim_order_base_h_included_

#include <vector>
#include <marketsim/common_types.h>

namespace marketsim
{
	struct VolumeHolder 
	{
		VolumeHolder(Volume v) : volume(v) {}

        bool filled() const 
        {
            return volume == 0;
        }

        template <class OtherOrder>
            void onMatched(PriceVolume const & x, OtherOrder const &)
        {
            volume -= x.volume;
        }

        void onCancelled()
        {
            volume = 0;
        }

        bool cancelled() const 
        {
            return volume == 0;
        }

        Volume		volume;
	};

    template <Side side>	
        struct side_is
        {
            typedef side_tag<side>  order_side_tag;

            static Price calculatePnL(PriceVolume const & x)
            {
                return side == Buy ? -x.PnL_Raw() : +x.PnL_Raw();
            }

            static Volume calculateQuantity(Volume v)
            {
                return side == Buy ? +v : -v;
            }
        };

	template <Side side>	
        struct PriceHolder : side_is<side>
	{
		PriceHolder(Price p) : price(p) {}

        struct OrderedByPrice 
        {
            bool operator () (PriceHolder const & lhs, PriceHolder const & rhs)
            {
                return side == Buy ? lhs.price < rhs.price : lhs.price > rhs.price;
            }
        };

		Price	price;  // const!!!
	};

    template <Side side, class Derived>
        struct LimitOrderBase : VolumeHolder, PriceHolder<side>
    {
        explicit LimitOrderBase(PriceVolume const & pv) : PriceHolder<side>(pv.price), VolumeHolder(pv.volume) {}

        typedef Derived derived_t;

        DECLARE_ARROW(Derived);

        typedef limit_order_tag category;

        template <class T>
        bool canBeMatched(LimitOrderBase<side == Buy ? Sell : Buy, T> const & best_limit, PriceVolume & matching) const 
        {
            if ((side == Buy && price >= best_limit.price)
                ||(side == Sell&& price <= best_limit.price))
            {
                matching.price = best_limit.price;
                matching.volume = std::min(volume, best_limit.volume);
                return true;
            }
            return false;
        }

        friend std::ostream & operator << (std::ostream & out, LimitOrderBase const & p)
        {
            return out << "(" << p.volume << " @ " << p.price << ")";
        }
    };

    template <class T>  
        std::ostream & operator << (std::ostream &out, boost::intrusive_ptr<T> const & p)
        {
            return out << *p;
        }

    template <Side side, class Derived>    
        struct MarketOrderBase : VolumeHolder, side_is<side>
    {
        explicit MarketOrderBase(Volume v) : VolumeHolder(v) {}

        typedef Derived derived_t;

        typedef market_order_tag category;

        typedef MarketOrderBase base;

        DECLARE_ARROW(Derived);

        template <class T>
        bool canBeMatched(LimitOrderBase<side == Buy ? Sell : Buy, T> const & best_limit, PriceVolume & matching) const 
        {
            matching.volume = std::min(volume, best_limit.volume);
            matching.price = best_limit.price;
            return true;
        }
    };
}

#endif