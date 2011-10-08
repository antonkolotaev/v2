#ifndef _marketsim_order_volume_holder_h_included_
#define _marketsim_order_volume_holder_h_included_

#include <marketsim/common/PriceVolume.h>

namespace marketsim {
namespace order     {
    /// Base class for orders encapsulating logic related to the volume of the order
    /// Holds volume of an order (market or limit)
    /// If order volume is 0 it is considered either as filled or cancelled
	struct VolumeHolder 
	{
		VolumeHolder(Volume v) : volume_(v) {}

        /// \return true iff order is filled
        bool filled() const 
        {
            return volume_ == 0;
        }

        /// reaction on successful matching with another order: the volume is diminished
        /// \param x - parameters of the trade
        /// \param OtherOrder - other order type; not uesd here
        template <typename OtherOrder>
            void onMatched(PriceVolume const & x, OtherOrder const &)
        {
            assert(volume_ >= x.volume);
            volume_ -= x.volume;
        }

        /// makes an order cancelled, i.e. puts its volume to 0
        void onCancelled()
        {
            volume_ = 0;
        }

        /// \return true iff order is cancelled (we don't distinguish cases of filled and cancelled orders)
        bool cancelled() const 
        {
            return volume_ == 0;
        }

        /// \return volume of the order
        Volume getVolume() const 
        {
            return volume_;
        }

#ifdef MARKETSIM_BOOST_PYTHON
        template <typename T>
            static void py_visit(T & c)
            {
                c.def_readonly("volume", &VolumeHolder::volume_);
            }
#endif
    private:
        Volume		volume_;
	};

}}

#endif