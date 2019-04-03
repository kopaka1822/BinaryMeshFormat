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
	auto res = m1.changeAttributes(Texcoord0, {});
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