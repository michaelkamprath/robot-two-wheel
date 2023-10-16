#include <unity.h>
#include "test_PointSequence.h"
#include "PointSequence.h"
#include "StringStream.h"

void test_Point_math(void) {
    Point p1(0, 0);
    Point p2(1, 0);
    Point p3(1, 1);

    TEST_ASSERT_EQUAL_DOUBLE(p1.distance(p2), 1.0);
    TEST_ASSERT_EQUAL_DOUBLE(p2.distance(p1), 1.0);
    TEST_ASSERT_FALSE(p1 == p2);
    TEST_ASSERT_TRUE(p1 != p2);
    TEST_ASSERT_TRUE(p1 + p2 == Point(1, 0));
    TEST_ASSERT_EQUAL_DOUBLE(-90, p1.absolute_bearing(p2));

    TEST_ASSERT_EQUAL_DOUBLE(p1.distance(p3), sqrt(2));
    TEST_ASSERT_EQUAL_DOUBLE(p3.distance(p1), sqrt(2));
    TEST_ASSERT_FALSE(p1 == p3);
    TEST_ASSERT_TRUE(p1 != p3);
    TEST_ASSERT_TRUE(p1 + p3 == Point(1, 1));
    TEST_ASSERT_EQUAL_DOUBLE(-45, p1.absolute_bearing(p3));

    TEST_ASSERT_EQUAL_DOUBLE(p2.distance(p3), 1.0);
    TEST_ASSERT_EQUAL_DOUBLE(p3.distance(p2), 1.0);
    TEST_ASSERT_FALSE(p2 == p3);
    TEST_ASSERT_TRUE(p2 != p3);
    TEST_ASSERT_TRUE(p2 + p3 == Point(2, 1));
    TEST_ASSERT_EQUAL_DOUBLE(0, p2.absolute_bearing(p3));

}

void test_PointSequence(void) {
    PointSequence ps1;

    TEST_ASSERT_EQUAL_UINT16(0, ps1.size());
    TEST_ASSERT_EQUAL_UINT16(10, ps1.capacity());

    ps1.add(Point(0, 0));
    ps1.add(Point(1, 0));
    ps1.add(Point(1, 1));

    TEST_ASSERT_EQUAL(3, ps1.size());
    TEST_ASSERT_EQUAL(10, ps1.capacity());

    StringStream ss1;
    ps1.write_to_stream(ss1);
    TEST_ASSERT_EQUAL_STRING("[(0,0),(1,0),(1,1)]", ss1.to_string().c_str());

}