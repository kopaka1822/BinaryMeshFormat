#include "pch.h"

#define TestSuite BinaryMeshTest

TEST(TestSuite, SummedIncies)
{
	using BinaryMesh = BinaryMesh16;

	const std::vector<float> vertices = {
		0.0f, 0.0f, 0.0f, // vertex 1
		1.0f, 0.0f, 1.0f, // vertex 2
		0.0f, 1.0f, 0.0f, // vertex 3
		1.0f, 0.0f, 2.0f, // vertex 4
		0.0f, 0.0f, 2.0f, // vertex 5
		1.0f, 1.0f, 2.0f, // vertex 6
	};
	const std::vector<uint16_t> indices = {
		0, 1, 2, // triangle 1
		0, 1, 2, // triangle 2
	};
	const std::vector<Shape> shapes = {
		Shape{0, 3, 0, 3, /*0, 1,*/ 1}, // shape
		Shape{3, 3, 3, 3, /*0, 1,*/ 2}, // shape
	};

	BinaryMesh m1(Position, vertices, indices, shapes);//, getIdentityVec(1));
	m1.generateBoundingBoxes();
	EXPECT_NO_THROW(m1.verify());

	auto si = m1.getSummedIndices();
	std::vector<uint32_t> expected{0, 1, 2, 3, 4, 5};
	EXPECT_EQ(si, expected);
}


TEST(TestSuite, MaterialPerTriangle)
{
	using BinaryMesh = BinaryMesh16;

	const std::vector<float> vertices = {
		0.0f, 0.0f, 0.0f, // vertex 1
		1.0f, 0.0f, 1.0f, // vertex 2
		0.0f, 1.0f, 0.0f, // vertex 3
		1.0f, 0.0f, 2.0f, // vertex 4
		0.0f, 0.0f, 2.0f, // vertex 5
		1.0f, 1.0f, 2.0f, // vertex 6
	};
	const std::vector<uint16_t> indices = {
		0, 1, 2, // triangle 1
		2, 1, 0, // triangle 2
		0, 1, 2, // triangle 3
	};
	const std::vector<Shape> shapes = {
		Shape{0, 6, 0, 3, /*0, 1,*/ 1}, // shape
		Shape{6, 3, 3, 3, /*0, 1,*/ 2}, // shape
	};

	BinaryMesh m1(Position, vertices, indices, shapes);//, getIdentityVec(1));
	m1.generateBoundingBoxes();
	EXPECT_NO_THROW(m1.verify());

	auto mids = m1.getMaterialIdPerTriangle();
	std::vector<uint32_t> expected{ 1, 1, 2 };
	EXPECT_EQ(mids, expected);
}