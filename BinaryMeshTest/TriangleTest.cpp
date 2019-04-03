#include "pch.h"

#define TestSuite TriangleTest

TEST(TestSuite, Rotate)
{
	uint32_t i0 = 0;
	uint32_t i1 = 1;
	uint32_t i2 = 2;
	std::array<uint32_t*, 3> data = { &i0, &i1, &i2 };

	IndexTriangle itri = {data};

	EXPECT_EQ(itri.index[0], &i0);
	EXPECT_EQ(itri.index[1], &i1);
	EXPECT_EQ(itri.index[2], &i2);

	itri.rotateLeft();
	EXPECT_EQ(itri.index[0], &i1);
	EXPECT_EQ(itri.index[1], &i2);
	EXPECT_EQ(itri.index[2], &i0);

	itri.rotateRight();
	EXPECT_EQ(itri.index[0], &i0);
	EXPECT_EQ(itri.index[1], &i1);
	EXPECT_EQ(itri.index[2], &i2);

}