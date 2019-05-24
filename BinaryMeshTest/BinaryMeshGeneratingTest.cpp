#include "pch.h"

#define TestSuite BinaryMeshTest

TEST(TestSuite, GenerateRemove)
{
	using BinaryMesh = BinaryMesh32;

	const std::vector<float> vertices = {
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // vertex 1
		1.0f, 0.0f, 1.0f, 0.1f, 0.2f, // vertex 2
		0.0f, 1.0f, 0.0f, 0.5f, 0.6f, // vertex 3
		1.0f, 0.0f, 2.0f, 0.7f, 0.9f, // vertex 4
	};
	const std::vector<uint32_t> indices = {
		0, 1, 2, // triangle 1
		1, 2, 3, // triangle 2
	};
	const std::vector<Shape> shapes = {
		Shape{0, 6, 0, 4, /*0, 1,*/ 2}, // shape
	};

	BinaryMesh m1(Texcoord0 | Position, vertices, indices, shapes);//, getIdentityVec(1));
	m1.generateBoundingBoxes();
	EXPECT_NO_THROW(m1.verify());

	// removing texcoords
	BinaryMesh res(m1);
	res.changeAttributes(Texcoord0, {});

	EXPECT_EQ(res.getAttributes(), Texcoord0);
	EXPECT_EQ(res.getIndices(), indices);
	EXPECT_EQ(res.getShapes().size(), shapes.size());

	// test for vertices without position
	EXPECT_EQ(res.getVertices(), std::vector<float>({
		0.0f, 0.0f, // vertex 1
		0.1f, 0.2f, // vertex 2
		0.5f, 0.6f, // vertex 3
		0.7f, 0.9f, // vertex 4
	}));

	EXPECT_NO_THROW(res.verify());
}

TEST(TestSuite, RemoveDuplicates)
{
	using BinaryMesh = BinaryMesh32;

	const std::vector<float> vertices = {
		0.0f, 1.0f, // vertex 0
		1.0f, 0.0f, // vertex 1
		1.0f, 0.0f, // vertex 2 (duplicate)
		0.0f, 1.0f, // vertex 3 (duplicate)
	};
	const std::vector<uint32_t> indices = {
		0, 1, 2, // triangle 1
		1, 2, 3, // triangle 2
	};
	const std::vector<Shape> shapes = {
		Shape{0, 6, 0,4, /*0, 1,*/ 2}, // shape
	};

	BinaryMesh m1(Texcoord0, vertices, indices, shapes);//, getIdentityVec(1));
	
	EXPECT_NO_THROW(m1.verify());
	m1.removeDuplicateVertices();

	const std::vector<float> expectedVertices = {
		0.0f, 1.0f, // vertex 0
		1.0f, 0.0f, // vertex 1
	};
	const std::vector<uint32_t> expectedIndices = {
		0, 1, 1, // triangle 1
		1, 1, 0, // triangle 2
	};
	const Shape expectedShape = { 0, 6, 0, 2, /*0, 1,*/ 2 };

	EXPECT_EQ(m1.getVertices(), expectedVertices);
	EXPECT_EQ(m1.getIndices(), expectedIndices);
	EXPECT_EQ(memcmp(m1.getShapes().data(), &expectedShape, sizeof(expectedShape)), 0);
	EXPECT_NO_THROW(m1.verify());
}

TEST(TestSuite, RemoveUnusedVertices)
{
	using BinaryMesh = BinaryMesh32;

	const std::vector<float> vertices = {
		0.0f, 1.0f, // vertex 0
		2.0f, 3.0f, // vertex 1
		4.0f, 5.0f, // vertex 2
		6.0f, 7.0f, // vertex 3
		8.0f, 9.0f, // vertex 4
		0.1f, 0.2f, // vertex 5
	};
	const std::vector<uint32_t> indices = {
		0, 1, 3, // triangle 1
		1, 0, 3, // triangle 2
		1, 3, 5, // triangle 3
	};
	const std::vector<Shape> shapes = {
		Shape{0, 9, 0, 6, /*0,1,*/2}, // shape
	};

	BinaryMesh m1(Texcoord0, vertices, indices, shapes);//, getIdentityVec(1));
	
	EXPECT_NO_THROW(m1.verify());
	m1.removeUnusedVertices();

	// 2 and 4 were unused
	const std::vector<float> expectedVertices = {
		0.0f, 1.0f, // vertex 0
		2.0f, 3.0f, // vertex 1
		//4.0f, 5.0f, // vertex 2
		6.0f, 7.0f, // vertex 3
		//8.0f, 9.0f, // vertex 4
		0.1f, 0.2f, // vertex 5
	};
	const std::vector<uint32_t> expectedIndices = {
		0, 1, 2, // triangle 1
		1, 0, 2, // triangle 2
		1, 2, 3, // triangle 3
	};
	const Shape expectedShape = { 0, 9, 0, 4, /*0, 1,*/ 2 };

	EXPECT_EQ(m1.getVertices(), expectedVertices);
	EXPECT_EQ(m1.getIndices(), expectedIndices);
	EXPECT_EQ(memcmp(m1.getShapes().data(), &expectedShape, sizeof(expectedShape)), 0);
	EXPECT_NO_THROW(m1.verify());
}

TEST(TestSuite, Force16Bit)
{
	using BinaryMesh = BinaryMesh32;

	std::vector<float> vertices;
	const std::vector<uint32_t> indices = {
		0, 1, 3, // 1st partition
		70000, 0, 70001, // mixed => 2nd partition
		70000, 70001, 70002, // 2nd partition
	};
	vertices.assign(70003 * 3, -1.0f);
	// write special values to the used vertices
	vertices[0] = 0.0f;
	vertices[3 * 1] = 1.0f;
	vertices[3 * 3] = 2.0f;
	vertices[3 * 70000] = 3.0f;
	vertices[3 * 70001] = 4.0f;
	vertices[3 * 70002] = 5.0f;

	const std::vector<Shape> shapes = {
		Shape{0, 9, 0, 70003, /*0,1,*/2}, // shape
	};

	BinaryMesh m1(Position, vertices, indices, shapes);//, getIdentityVec(1));
	m1.generateBoundingBoxes();

	EXPECT_NO_THROW(m1.verify());
	
	auto res = m1.force16BitIndices();
	EXPECT_EQ(res.size(), 2);

	for (auto& r : res)
		EXPECT_NO_THROW(r.verify());

	// test index counts
	EXPECT_EQ(res[0].getIndices().size(), 3);
	EXPECT_EQ(res[1].getIndices().size(), 6);
	// test vertices
	EXPECT_EQ(res[0].getVertices()[0], vertices[0]);
	EXPECT_EQ(res[0].getVertices()[3 * 1], vertices[3 * 1]);
	EXPECT_EQ(res[0].getVertices()[3 * 2], vertices[3 * 3]);

	EXPECT_EQ(res[1].getVertices()[0], vertices[0]);
	EXPECT_EQ(res[1].getVertices()[3 * 1], vertices[3 * 70000]);
	// test indices
	EXPECT_EQ(res[1].getIndices()[0], 1);
}