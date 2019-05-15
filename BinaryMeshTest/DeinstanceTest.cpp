#include "pch.h"

#define TestSuite BinaryMeshTest

TEST(TestSuite, InstanceMerge)
{
	const std::vector<float> vertices1 = {
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // vertex 1
		1.0f, 0.0f, 1.0f, 0.1f, 0.2f, // vertex 2
		2.0f, 1.0f, 0.0f, 0.5f, 0.6f, // vertex 3
		3.0f, 0.0f, 2.0f, 0.7f, 0.9f, // vertex 4
	};
	const std::vector<uint32_t> indices1 = {
		0, 1, 2, // triangle 1
		1, 2, 3, // triangle 2
	};
	const std::vector<BinaryMesh::Shape> shapes1 = {
		BinaryMesh::Shape{0, 6, 0, 4, 0, 1, 2}, // shape 1
	};
	const std::vector<glm::vec3> instances1 = {
		glm::vec3(0.0f),
	};

	std::vector<float> vertices2 = vertices1;
	glm::vec3 transVec = glm::vec3(10.0f, -200.0f, 1.0f);
	const auto stride = getAttributeElementStride(Position | Texcoord0);
	for(size_t i = 0; i < vertices2.size(); i += stride)
	{
		auto res = transVec + glm::vec3(vertices2[i], vertices2[i + 1], vertices2[i + 2]);
		vertices2[i] = res.x;
		vertices2[i + 1] = res.y;
		vertices2[i + 2] = res.z;
	}

	std::vector<BinaryMesh> meshes;
	meshes.emplace_back(Position | Texcoord0, vertices1, indices1, shapes1, instances1);
	meshes.emplace_back(Position | Texcoord0, vertices2, indices1, shapes1, instances1);
	EXPECT_NO_THROW(meshes[0].verify());
	EXPECT_NO_THROW(meshes[1].verify());

	BinaryMesh::deinstanceShapes(meshes);
	
	// should be succesfull deinstance
	EXPECT_EQ(meshes.size(), 1);
	EXPECT_EQ(meshes[0].getShapes().size(), 1);
	EXPECT_EQ(meshes[0].getInstanceTransforms().size(), 2);
	auto transVec2 = meshes[0].getInstanceTransforms()[1];
	// transform matrices
	
	for (glm::length_t i = 0; i < 3; ++i)
		EXPECT_LT(std::abs(transVec[i] - transVec2[i]), 0.0001f);

	EXPECT_NO_THROW(meshes[0].verify());
}