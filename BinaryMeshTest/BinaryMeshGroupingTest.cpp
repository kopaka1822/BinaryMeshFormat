#include "pch.h"

#define TestSuite BinaryMeshTest

TEST(TestSuite, Split)
{
	const std::vector<float> vertices = {
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // vertex 0
	1.0f, 0.0f, 1.0f, 0.1f, 0.2f, // vertex 1
	2.0f, 1.0f, 0.0f, 0.5f, 0.6f, // vertex 2
	3.0f, 0.0f, 2.0f, 0.7f, 0.9f, // vertex 3
	4.0f, 0.0f, 2.0f, 0.7f, 0.9f, // vertex 4
	4.0f, 8.0f, 2.0f, 0.7f, 0.9f, // vertex 5
	9.0f, 0.0f, 2.0f, 0.7f, 0.9f, // vertex 6
	};
	const std::vector<uint32_t> indices = {
		0, 1, 2, // triangle 1 shape 1
		1, 2, 3, // triangle 2 shape 1
		1, 0, 2, // triangle 3 shape 2 (indices start by 0 again with offset 4)
	};
	const std::vector<glm::mat4> instances = {
		glm::mat4(1.0f),
		glm::mat4(2.0f),
		glm::mat4(3.0f),
	};
	const std::vector<BinaryMesh::Shape> shapes = {
		BinaryMesh::Shape{0, 6, 0, 4, 0, 1, 2}, // shape 1
		BinaryMesh::Shape{6, 3, 4, 3, 1, 2, 7}, // shape 2
	};

	BinaryMesh baseMesh(Texcoord0 | Position, vertices, indices, shapes, instances);
	EXPECT_NO_THROW(baseMesh.verify());

	const auto splitted = baseMesh.splitShapes();
	EXPECT_EQ(splitted.size(), 2);
	// check indices
	EXPECT_EQ(splitted[0].getIndices(), std::vector<uint32_t>(indices.begin(), indices.begin() + 6));
	EXPECT_EQ(splitted[1].getIndices(), std::vector<uint32_t>(indices.begin() + 6, indices.end()));

	// check vertices
	const auto stride = getAttributeElementStride(baseMesh.getAttributes());
	EXPECT_EQ(splitted[0].getVertices(), std::vector<float>(vertices.begin(), vertices.begin() + stride * shapes[0].vertexCount));
	EXPECT_EQ(splitted[1].getVertices(), std::vector<float>(vertices.begin() + stride * shapes[1].vertexOffset, vertices.end()));

	// check shapes count
	EXPECT_EQ(splitted[0].getShapes().size(), 1);
	EXPECT_EQ(splitted[1].getShapes().size(), 1);

	// check instances
	EXPECT_EQ(splitted[0].getInstanceTransforms(), std::vector<glm::mat4>(instances.begin(), instances.begin() + shapes[0].instanceCount));
	EXPECT_EQ(splitted[1].getInstanceTransforms(), std::vector<glm::mat4>(instances.begin() + shapes[1].instanceOffset, instances.end()));

	// adjusted offset
	EXPECT_EQ(splitted[1].getShapes()[0].indexOffset, 0);
	EXPECT_EQ(splitted[1].getShapes()[0].vertexOffset, 0);
	EXPECT_EQ(splitted[1].getShapes()[0].instanceOffset, 0);
	EXPECT_EQ(splitted[1].getShapes()[0].materialId, 7);
	for(const auto& s : splitted)
	{
		EXPECT_NO_THROW(s.verify());
	}
}

TEST(TestSuite, Merge)
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
		BinaryMesh::Shape{0, 6, 0, 4, 0, 2, 2}, // shape 1
	};
	const std::vector<glm::mat4> instances1 = {
		glm::mat4(1.0f),
		glm::mat4(2.0f),
	};

	const std::vector<float> vertices2 = {
		2.0f, 1.0f, 0.0f, 0.5f, 0.6f, // vertex 1
		3.0f, 0.0f, 2.0f, 0.7f, 0.9f, // vertex 2
		4.0f, 0.0f, 2.0f, 0.7f, 0.9f, // vertex 3
	};
	const std::vector<uint32_t> indices2 = {
		1, 0, 2, // triangle 1
	};
	const std::vector<BinaryMesh::Shape> shapes2 = {
		BinaryMesh::Shape{0, 3, 0, 3, 0, 1, 7}, // shape 1
	};
	const std::vector<glm::mat4> instances2 = {
		glm::mat4(3.0f),
	};

	std::vector<BinaryMesh> meshes;
	meshes.emplace_back(Position | Texcoord0, vertices1, indices1, shapes1, instances1);
	meshes.emplace_back(Position | Texcoord0, vertices2, indices2, shapes2, instances2);
	EXPECT_NO_THROW(meshes[0].verify());
	EXPECT_NO_THROW(meshes[1].verify());

	auto merged = BinaryMesh::mergeShapes(meshes);

	// check count
	EXPECT_EQ(merged.getShapes().size(), 2);

	// adjusted offset
	const auto stride = getAttributeElementStride(merged.getAttributes());

	EXPECT_EQ(merged.getShapes()[0].indexOffset, 0);
	EXPECT_EQ(merged.getShapes()[0].vertexOffset, 0);
	EXPECT_EQ(merged.getShapes()[0].indexCount, indices1.size());
	EXPECT_EQ(merged.getShapes()[0].vertexCount, vertices1.size() / stride);
	EXPECT_EQ(merged.getShapes()[0].materialId, shapes1[0].materialId);

	EXPECT_EQ(merged.getShapes()[1].indexOffset, indices1.size());
	EXPECT_EQ(merged.getShapes()[1].vertexOffset, vertices1.size() / stride);
	EXPECT_EQ(merged.getShapes()[1].indexCount, indices2.size());
	EXPECT_EQ(merged.getShapes()[1].vertexCount, vertices2.size() / stride);
	EXPECT_EQ(merged.getShapes()[1].materialId, shapes2[0].materialId);

	// indices
	EXPECT_EQ(merged.getIndices().size(), indices1.size() + indices2.size());
	// test some index values
	// mesh 1s
	EXPECT_TRUE(std::equal(merged.getIndices().begin(), merged.getIndices().begin() + indices1.size(), indices1.begin()));
	EXPECT_TRUE(std::equal(merged.getIndices().begin() + indices1.size(), merged.getIndices().end(), indices2.begin()));

	// check vertices
	EXPECT_TRUE(std::equal(merged.getVertices().begin(), merged.getVertices().begin() + vertices1.size(), vertices1.begin()));
	EXPECT_TRUE(std::equal(merged.getVertices().begin() + vertices1.size(), merged.getVertices().end(), vertices2.begin()));

	// check instances
	EXPECT_TRUE(std::equal(merged.getInstanceTransforms().begin(), merged.getInstanceTransforms().begin() + instances1.size(), instances1.begin()));
	EXPECT_TRUE(std::equal(merged.getInstanceTransforms().begin() + instances1.size(), merged.getInstanceTransforms().end(), instances2.begin()));

	EXPECT_NO_THROW(merged.verify());
}

TEST(TestSuite, MergeError)
{
	// incompatible attributes
	std::vector<BinaryMesh> meshes;
	meshes.emplace_back(Normal, std::vector<float>{}, std::vector<uint32_t>{}, std::vector<BinaryMesh::Shape>{}, getIdentityVec(0));
	meshes.emplace_back(Position, std::vector<float>{}, std::vector<uint32_t>{}, std::vector<BinaryMesh::Shape>{}, getIdentityVec(0));

	EXPECT_THROW(BinaryMesh::mergeShapes(meshes), std::runtime_error);
}