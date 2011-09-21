#include "stdafx.h"
#include "catch.hpp"

#include <marketsim/scheduler.h>
#include <marketsim/object_pool.h>

namespace marketsim  {
namespace {

    int g_EEE_counter = 0;

    struct EEE : EventHandler
    {
        EEE(object_pool<EEE> *s) : storage_(s), ID_(g_EEE_counter++)
        {
            schedule(5.);
            //std::cout << "created " << ID_ << std::endl;
        }

        ~EEE()
        {
            //std::cout << "destroyed " << ID_ << std::endl;
        }

        void process() {}

    protected:
        void on_released()
        {
            storage_->free(this);
        }
    private:
        object_pool<EEE>  * storage_;  
        int ID_;
    };

    struct A 
    {
        template <class T> void getX(T);
    };

    struct B 
    {
        struct Tag {};
        Tag getX(Tag) { return Tag(); }
    };

    struct C 
    {
        struct Tag {};
        Tag getX(Tag) { return Tag(); }
    };

    template <class Y, class Base>
        struct Mix : Base, Y
    {
        using Base::getX;
        using Y::getX;
    };

    struct P : Mix<B, Mix<C, A> > {};

    void f()
    {
        P p;

        C::Tag a = p.getX(C::Tag());
        B::Tag b = p.getX(B::Tag());
    }

    struct FFF : EventHandler
    {
        FFF() : counter_(10)
        {
            schedule(1.);
        }

        void on_released() {}

        void process()
        {
            new (storage_.alloc()) EEE(&storage_);

            if (--counter_ > 0)
                schedule(1.);
        }

        object_pool<EEE>    storage_;
        int                 counter_;
    };

    TEST_CASE("scheduler", "Testing scheduler events resided in a pool")
    {
        FFF fff;

        scheduler().workTill(100);

        scheduler().reset();
    }
}}