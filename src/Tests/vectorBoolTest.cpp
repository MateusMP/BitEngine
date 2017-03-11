#include <vector>

#include "bitengine\Common\VectorBool.h"

#include "gtest/gtest.h"

TEST(VectorBoolTest, ResizeInitWith0)
{
	BitEngine::VectorBool vec;
	vec.resize(1024);
	for (int i = 0; i < 1024; ++i){
		ASSERT_EQ(vec[i], false);
	}
}

TEST(VectorBoolTest, PushBackValue)
{
	BitEngine::VectorBool vec;
	vec.push_back(true);
	vec.push_back(0);
	vec.push_back(true);
	vec.push_back(false);
	vec.push_back(1);
	
	ASSERT_TRUE(vec[0]);
	ASSERT_FALSE(vec[1]);
	ASSERT_TRUE(vec[2]);
	ASSERT_FALSE(vec[3]);
	ASSERT_TRUE(vec[4]);
	ASSERT_EQ(5, vec.size());
}


TEST(VectorBoolTest, ResizeReduceNumElements)
{
	BitEngine::VectorBool vec;
	vec.resize(62);
	ASSERT_EQ(62, vec.size());
	vec.resize(5);
	ASSERT_EQ(5, vec.size());
}


TEST(VectorBoolTest, SetUnset)
{
	BitEngine::VectorBool vec;
	vec.resize(14);
	vec.set(13);
	ASSERT_TRUE(vec[13]);
	vec.unset(13);
	ASSERT_FALSE(vec[13]);
	for (int i = 0; i < 14; ++i) {
		ASSERT_FALSE(vec[i]);
	}
}

TEST(VectorBoolTest, TestBit)
{
	BitEngine::VectorBool vec;
	vec.resize(14);
	for (int i = 0; i < 14; ++i) {
		ASSERT_FALSE(vec.test(i));
	}
	
	vec.set(7);
	ASSERT_TRUE(vec.test(7));
}
