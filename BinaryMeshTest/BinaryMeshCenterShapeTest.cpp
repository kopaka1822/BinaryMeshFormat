#include "pch.h"
/*
#define TestSuite BinaryMeshTest

TEST(TestSuite, ConterShape)
{
	const std::vector<float> vertices = {
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // vertex 1
		1.0f, 0.0f, 0.0f, 0.1f, 0.2f, // vertex 2
		0.0f, 1.0f, 0.0f, 0.5f, 0.6f, // vertex 3
	};
	const std::vector<uint32_t> indices = {
		0, 1, 2, // triangle 1
	};
	const std::vector<BinaryMesh::Shape> shapes = {
		BinaryMesh::Shape{0, 3, 0, 3, 0, 1, 2}, // shape 1
	};
	const std::vector<glm::vec3> instances = {
		glm::vec3(0.0f),
	};

	BinaryMesh m(Position | Texcoord0, vertices, indices, shapes, instances);
	EXPECT_NO_THROW(m.verify());

	m.centerShapes();

	const std::vector<float> expectedVertices = {
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // vertex 1
	0.5f, -0.5f, 0.0f, 0.1f, 0.2f, // vertex 2
	-0.5f, 0.5f, 0.0f, 0.5f, 0.6f, // vertex 3
	};
	
	EXPECT_EQ(m.getVertices(), expectedVertices);
	auto mat = glm::vec3(0.5f, 0.5f, 0.0f);
	EXPECT_EQ(m.getInstanceTransforms()[0], mat);
}*/