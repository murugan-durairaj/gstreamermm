/*
 * test-query.cc
 *
 *  Created on: 5 sie 2013
 *      Author: loganek
 */

#include <gtest/gtest.h>
#include <gstreamermm.h>
#include <functional>

using namespace Gst;
using Glib::RefPtr;

template<typename QType>
void CreatingQueryTest(std::function<RefPtr<QType>()> create_method, QueryType type)
{
    RefPtr<QType> query = create_method();

    ASSERT_TRUE(query);
    ASSERT_EQ(type, query->get_query_type());
}

template<typename QType>
void CreatingQueryTestStatic(std::function<RefPtr<Query>()> create_method, QueryType type)
{
    RefPtr<Query> query = create_method();

    ASSERT_TRUE(query);
    ASSERT_EQ(type, query->get_query_type());

    RefPtr<QType> query_position = RefPtr<QType>::cast_static(query);

    ASSERT_TRUE(query_position);
}

TEST(QueryTest, CorrectCreatingQueryBuffering)
{
    CreatingQueryTestStatic<QueryBuffering>(
            std::bind(&Query::create_buffering, FORMAT_BUFFERS), QUERY_BUFFERING);
    CreatingQueryTest<QueryBuffering>(
            std::bind(&QueryBuffering::create, FORMAT_BUFFERS), QUERY_BUFFERING);
}

TEST(QueryTest, CorrectCreatingQueryPosition)
{
    CreatingQueryTestStatic<QueryPosition>(
            std::bind(&Query::create_position, FORMAT_PERCENT), QUERY_POSITION);
    CreatingQueryTest<QueryPosition>(
            std::bind(&QueryPosition::create, FORMAT_PERCENT), QUERY_POSITION);
}

TEST(QueryTest, CorrectCreatingQueryConvert)
{
    CreatingQueryTestStatic<QueryConvert>(
            std::bind(&Query::create_convert, FORMAT_PERCENT, 10, FORMAT_BYTES), QUERY_CONVERT);
    CreatingQueryTest<QueryConvert>(
            std::bind(&QueryConvert::create, FORMAT_PERCENT, 10, FORMAT_BYTES), QUERY_CONVERT);
}

TEST(QueryTest, CheckRefCountsDuringQueryCapsCreation)
{
  RefPtr<Caps> filter = Caps::create_from_string("video/x-raw");
  RefPtr<QueryCaps> query = QueryCaps::create(filter);

  RefPtr<Caps> caps = query->parse();
  ASSERT_EQ(caps, filter);
  ASSERT_EQ(3, caps->gobj()->mini_object.refcount); // - 1 (caps object) + 1 (filter object) + 1 (owned by query) = 3
}

TEST(QueryTest, CheckStoringAllocationParams)
{
  auto alloc_query = QueryAllocation::create(Caps::create_from_string("video/x-raw, format=RGB"), false);
  RefPtr<Allocator> allocator = Allocator::get_default_allocator(), allocator2;
  AllocationParams params, params2;
  params.init(); params.set_align(10);
  alloc_query->add_allocation_param(allocator, params);
  alloc_query->parse_nth_allocation_param(0, allocator2, params2);
  ASSERT_EQ(allocator, allocator2);
  ASSERT_EQ(params.get_align(), params2.get_align());
}


