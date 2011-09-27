
#include "catch.hpp"
#include <boost/tuple/tuple.hpp>

#include "orders.h"
#include <marketsim/scheduler.h>
#include <marketsim/object_pool.h>
#include <marketsim/order_queue.h>
#include <marketsim/order/in_pool.h>

namespace marketsim{
namespace {
    template <typename Base>
        struct WithExpiration : Base, EventHandler
        {
            template <typename T>
                WithExpiration(T const & x)
                    :   Base(boost::get<0>(x))
            {
                schedule(boost::get<1>(x));
            }

            typedef WithExpiration  base;   // for derived classes

            void process()
            {
                self()->onCancelled();
            }
        };

    int g_counter = 0;

    template <Side SIDE>
        struct LimitT :
            InPool<PlacedInPool,
                WithExpiration<
                    ExecutionHistory<
                        LimitOrderBase<SIDE, 
                            LimitT<SIDE> >
                    > > >
        {
            LimitT(PriceVolume const & x, TimeInterval life_time, object_pool<LimitT> * h, OrderQueue<boost::intrusive_ptr<LimitT> >  * queue)
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
            OrderQueue<boost::intrusive_ptr<LimitT> >  * queue_;
        };
                    
    typedef LimitT<Sell>    LimitSell;

    typedef boost::intrusive_ptr<LimitSell>                                      LimitSellPtr;


    TEST_CASE("order_expiry", "Testing orders with expiry time")
    {
        object_pool<LimitSell>      pool;

        OrderQueue<LimitSellPtr>    sell_orders;

        LimitSell * ls;

        sell_orders.push(ls = new (pool.alloc()) LimitSell(pv(100, 123), 15., &pool, &sell_orders));
        sell_orders.push(ls = new (pool.alloc()) LimitSell(pv(90, 978), 3., &pool, &sell_orders));

        scheduler().workTill(1.);

        REQUIRE(sell_orders.top()->volume == 978);
        REQUIRE(g_counter == 2);
        REQUIRE(!sell_orders.empty());
        scheduler().workTill(10.);

        sell_orders.push(ls = new (pool.alloc()) LimitSell(pv(91, 555), 1., &pool, &sell_orders));
        REQUIRE(g_counter == 2);
        REQUIRE(!sell_orders.empty());
        REQUIRE(sell_orders.top()->volume == 555);

        scheduler().workTill(12.);
        REQUIRE(g_counter == 1);
        REQUIRE(!sell_orders.empty());
        REQUIRE(sell_orders.top()->volume == 123);

        scheduler().workTill(20.);
        REQUIRE(g_counter == 0);
        REQUIRE(sell_orders.empty());

        scheduler().reset();
    }

}}