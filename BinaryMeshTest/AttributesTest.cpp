#include "pch.h"

#define TestSuite AttributeTest

TEST(TestSuite, Size)
{
	EXPECT_EQ(getAttributeElementCount(Normal), 3);
	EXPECT_EQ(getAttributeByteCount(Normal), 3 * sizeof(float));

	EXPECT_EQ(getAttributeElementCount(SIZE), 0);
}

TEST(TestSuite, OffsetAndStride)
{
	// normal is between position and texcoord
	EXPECT_EQ(getAttributeElementOffset(Position | Normal | Texcoord0, Normal), 3);
	EXPECT_EQ(getAttributeElementOffset(Position | Normal | Texcoord0, Texcoord0), 6);
	EXPECT_EQ(getAttributeElementStride(Position | Normal | Texcoord0), 8);

	EXPECT_EQ(getAttributeByteOffset(Position | Normal, Normal), 3 * sizeof(float));
	EXPECT_EQ(getAttributeByteStride(Position | Normal | Texcoord0), 8 * sizeof(float));
	
}