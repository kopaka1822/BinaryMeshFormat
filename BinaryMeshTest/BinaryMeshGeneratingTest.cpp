#include "pch.h"

#define TestSuite BinaryMeshTest

TEST(TestSuite, GenerateRemove)
{
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
	const std::vector<BinaryMesh::Shape> shapes = {
		BinaryMesh::Shape{0, 6, 2}, // shape
	};

	BinaryMesh m1(Texcoord0 | Position, vertices, indices, shapes);

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
}

TEST(TestSuite, RemoveDuplicates)
{
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
	const std::vector<BinaryMesh::Shape> shapes = {
		BinaryMesh::Shape{0, 6, 2}, // shape
	};

	BinaryMesh m1(Texcoord0, vertices, indices, shapes);
	m1.removeDuplicateVertices();

	const std::vector<float> expectedVertices = {
		0.0f, 1.0f, // vertex 0
		1.0f, 0.0f, // vertex 1
	};
	const std::vector<uint32_t> expectedIndices = {
		0, 1, 1, // triangle 1
		1, 1, 0, // triangle 2
	};

	EXPECT_EQ(m1.getVertices(), expectedVertices);
	EXPECT_EQ(m1.getIndices(), expectedIndices);
}