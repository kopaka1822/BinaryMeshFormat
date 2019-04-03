#include "pch.h"

#define TestSuite BinaryMeshTest

TEST(TestSuite, LoadSave)
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

	m1.saveToFile("LoadSaveTest.bmf");

	auto m2 = BinaryMesh::loadFromFile("LoadSaveTest.bmf");

	EXPECT_EQ(m1.getAttributes(), m2.getAttributes());
	EXPECT_EQ(m1.getVertices(), m2.getVertices());
	EXPECT_EQ(m1.getIndices(), m2.getIndices());

	EXPECT_EQ(m1.getShapes().size(), m2.getShapes().size());
	EXPECT_EQ(memcmp(m1.getShapes().data(), m2.getShapes().data(), m1.getShapes().size() * sizeof(BinaryMesh::Shape)), 0);

}