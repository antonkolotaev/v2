
#include "catch.hpp"
#include <boost/tuple/tuple.hpp>

#include "orders.h"
#include <marketsim/scheduler.h>
#include <marketsim/object_pool.h>
#include <marketsim/order_queue/order_queue.h>
#include <marketsim/order/in_pool.h>

namespace marketsim{
namespace {

    namespace order 
    {
        using namespace marketsim::order;

        template <typename Base>
            struct WithExpiration : Base
            {
                template <typename T>
                    WithExpiration(T const & x)
                        :   Base(boost::get<0>(x))
                        ,   timeout_(*this, &WithExpiration::process, boost::get<1>(x))
                {
                }

                typedef WithExpiration  base;   // for derived classes

                using Base::cancelled;
                using Base::derived_t;
                using Base::self;

                void process()
                {
                    this->self()->onCancelled();
                }
            private:
                TimeOut<WithExpiration> timeout_;
            };

        int g_counter = 0;

        template <Side SIDE>
            struct LimitT :
                InPool          <PlacedInPool,
                WithExpiration  <
                ExecutionHistory<
                LimitOrderBase  <SIDE, 
                RefCounted      <
                derived_is      <
                LimitT          <SIDE> 
                > > > > > >
            {
               typedef 
                   InPool          <PlacedInPool,
                   WithExpiration  <
                   ExecutionHistory<
                   LimitOrderBase  <SIDE, 
                   RefCounted      <
                   derived_is      <
                   LimitT          <SIDE> 
                   > > > > > >
                  base;
                                 
                LimitT(PriceVolume const & x, TimeInterval life_time, object_pool<LimitT> * h, order_queue::OrderQueue<boost::intrusive_ptr<LimitT> >  * queue)
                    :   base(boost::make_tuple(boost::make_tuple(x, life_time), h)), queue_(queue)
                {
                    ++g_counter;
    //                 std::cout 
    //                     << "[" << scheduler().currentTime() << "]\t"
    //                     << "Created " << x.volume << "@" << x.price << " to be destroyed at " << this->getActionTime() << std::endl;
                }

                void onCancelled()
                {
                    base::onCancelled();
                    queue_->onOrderCancelled(this);
                }

                ~LimitT()
                {
                    --g_counter;
    //                 std::cout 
    //                     << "[" << scheduler().currentTime() << "]\t"
    //                     << "Destroyed " << volume << "@" << price << " with killtime at " << this->getActionTime() << std::endl;
                }
            private:
                order_queue::OrderQueue<boost::intrusive_ptr<LimitT> >  * queue_;
            };
            
            //template <Side SIDE> void intrusive_ptr_add_ref(LimitT<SIDE> *p) { p->add_ref(); }
            //template <Side SIDE> void intrusive_ptr_release(LimitT<SIDE> *p) { p->release(); }
    }
    

    typedef order::LimitT<Sell>    LimitSell;

    typedef boost::intrusive_ptr<LimitSell>                                      LimitSellPtr;


    TEST_CASE("order_expiry", "Testing orders with expiry time")
    {
        Scheduler                   scheduler;
        object_pool<LimitSell>      pool;

        order_queue::OrderQueue<LimitSellPtr>    sell_orders;

        LimitSell * ls;

        using order::g_counter;

        sell_orders.push(ls = new (pool.alloc()) LimitSell(pv(100, 123), 15., &pool, &sell_orders));
        sell_orders.push(ls = new (pool.alloc()) LimitSell(pv(90, 978), 3., &pool, &sell_orders));

        scheduler.workTill(1.);

        assert(sell_orders.top()->getVolume() == 978);
        assert(g_counter == 2);
        assert(!sell_orders.empty());
        scheduler.workTill(10.);

        sell_orders.push(ls = new (pool.alloc()) LimitSell(pv(91, 555), 1., &pool, &sell_orders));
        assert(g_counter == 2);
        assert(!sell_orders.empty());
        assert(sell_orders.top()->getVolume() == 555);

        scheduler.workTill(12.);
        assert(g_counter == 1);
        assert(!sell_orders.empty());
        assert(sell_orders.top()->getVolume() == 123);

        scheduler.workTill(20.);
        assert(g_counter == 0);
        assert(sell_orders.empty());
    }

}}
