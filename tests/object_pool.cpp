
#include "catch.hpp"

#include <marketsim/object_pool.h>

namespace marketsim {
namespace {

    int g_TestInstances = 0;

    struct Test { Test(int x): x(x) { ++g_TestInstances; } int x; ~Test() { x = 0xf00d; --g_TestInstances; } };

    TEST_CASE("object_pool", "Object pool operations")
    {
        assert(g_TestInstances == 0);
        {
            object_pool<Test>    pool;

            Test *p1 = new (pool.alloc()) Test(12);
            Test *p2 = new (pool.alloc()) Test(34);
            pool.free(p1);
            pool.free(p2);
            assert(p1->x == 0xf00d);
            assert(p2->x == 0xf00d);
            Test *p3 = new (pool.alloc()) Test(56);
            Test *p4 = new (pool.alloc()) Test(12);
            Test *p5 = new (pool.alloc()) Test(34);
            assert(p3->x == 56);
            assert(p4->x == 12);
            assert(p5->x == 34);
            assert(g_TestInstances == 3);
        }
        assert(g_TestInstances == 0);
    }
}}