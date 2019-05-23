#include "pch.h"

#define TestSuite VertexTest

TEST(TestSuite, RefCtor)
{
	float data1[] = { 10.0f, 0.0f, 2.0f, 0.0f, 1.0f, 0.0f };

	RefVertex v(Position | Normal, data1);

	// address should match (because ref)
	EXPECT_EQ(v.data(), data1);
}

TEST(TestSuite, ValueCtor)
{
	float data1[] = { 10.0f, 0.0f, 2.0f, 0.0f, 1.0f, 0.0f };

	ValueVertex v(Position | Normal, data1);

	EXPECT_NE(data1, v.data());
	for(size_t i = 0; i < std::size(data1); ++i)
	{
		EXPECT_EQ(data1[i], v.data()[i]);
	}
}

TEST(TestSuite, Conversion)
{
	float data1[] = { 10.0f, 0.0f, 2.0f, 0.5f, 0.25f };
	float data2[] = { 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };

	RefVertex v1(Position | Texcoord0, data1);
	RefVertex v2(Normal | Texcoord0, data2);

	// normals should be same. texcoords should be overwritten
	v1.copyAttributesTo(v2);
	// normal
	EXPECT_EQ(data2[0], 0.0f);
	EXPECT_EQ(data2[1], 1.0f);
	EXPECT_EQ(data2[2], 0.0f);
	// texcoord
	EXPECT_EQ(data2[3], 0.5f);
	EXPECT_EQ(data2[4], 0.25f);
}

TEST(TestSuite, Get)
{
	float data1[] = { 10.0f, 0.0f, 2.0f, 0.5f, 0.25f, 2.0f, 4.0f };

	RefVertex v(Position | Texcoord0 | Texcoord2, data1);

	EXPECT_EQ(v.get(Position)[0], data1[0]);
	EXPECT_EQ(v.get(Position)[1], data1[1]);
	EXPECT_EQ(v.get(Position)[2], data1[2]);

	EXPECT_EQ(v.get(Texcoord0)[0], data1[3]);
	EXPECT_EQ(v.get(Texcoord0)[1], data1[4]);

	EXPECT_EQ(v.get(Texcoord2)[0], data1[5]);
	EXPECT_EQ(v.get(Texcoord2)[1], data1[6]);
}

TEST(TestSuite, Set)
{
	const float data1[] = { 10.0f, 0.0f, 2.0f, 0.5f, 0.25f, 2.0f, 4.0f };
	ValueVertex v(Position | Texcoord0 | Texcoord2, data1);

	const float newTex[] = { -1.0f, -2.0f };
	v.set(Texcoord0, newTex);

	EXPECT_EQ(v.get(Texcoord0)[0], newTex[0]);
	EXPECT_EQ(v.get(Texcoord0)[1], newTex[1]);
}
