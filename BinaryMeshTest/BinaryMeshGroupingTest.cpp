#include "pch.h"

#define TestSuite BinaryMeshTest

TEST(TestSuite, Split)
{
	const std::vector<float> vertices = {
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // vertex 1
	1.0f, 0.0f, 1.0f, 0.1f, 0.2f, // vertex 2
	2.0f, 1.0f, 0.0f, 0.5f, 0.6f, // vertex 3
	3.0f, 0.0f, 2.0f, 0.7f, 0.9f, // vertex 4
	4.0f, 0.0f, 2.0f, 0.7f, 0.9f, // vertex 5
	};
	const std::vector<uint32_t> indices = {
		0, 1, 2, // triangle 1 shape 1
		1, 2, 3, // triangle 2 shape 1
		3, 2, 4, // triangle 1 shape 2
	};
	const std::vector<BinaryMesh::Shape> shapes = {
		BinaryMesh::Shape{0, 6, 2}, // shape 1
		BinaryMesh::Shape{6, 3, 7}, // shape 2
	};

	BinaryMesh baseMesh(Texcoord0 | Position, vertices, indices, shapes);
	EXPECT_NO_THROW(baseMesh.verify());

	const auto splitted = baseMesh.splitShapes();
	EXPECT_EQ(splitted.size(), 2);
	// check indices
	EXPECT_EQ(splitted[0].getIndices(), std::vector<uint32_t>(indices.begin(), indices.begin() + 6));
	EXPECT_EQ(splitted[1].getIndices(), std::vector<uint32_t>({1, 0, 2})); // indices will always start at 0

	// check vertices
	const auto stride = getAttributeElementStride(baseMesh.getAttributes());
	EXPECT_EQ(splitted[0].getVertices(), std::vector<float>(vertices.begin(), vertices.begin() + stride * 4));
	EXPECT_EQ(splitted[1].getVertices(), std::vector<float>(vertices.begin() + stride * 2, vertices.end()));

	// check shapes count
	EXPECT_EQ(splitted[0].getShapes().size(), 1);
	EXPECT_EQ(splitted[1].getShapes().size(), 1);

	// adjusted offset
	EXPECT_EQ(splitted[1].getShapes()[0].indexOffset, 0);
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
		BinaryMesh::Shape{0, 6, 2}, // shape 1
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
		BinaryMesh::Shape{0, 3, 7}, // shape 1
	};

	std::vector<BinaryMesh> meshes;
	meshes.emplace_back(Position | Texcoord0, vertices1, indices1, shapes1);
	meshes.emplace_back(Position | Texcoord0, vertices2, indices2, shapes2);
	EXPECT_NO_THROW(meshes[0].verify());
	EXPECT_NO_THROW(meshes[1].verify());

	auto merged = BinaryMesh::mergeShapes(meshes);

	// check count
	EXPECT_EQ(merged.getShapes().size(), 2);

	// adjusted offset
	EXPECT_EQ(merged.getShapes()[0].indexOffset, 0);
	EXPECT_EQ(merged.getShapes()[1].indexOffset, indices1.size());

	// indices
	EXPECT_EQ(merged.getIndices().size(), indices1.size() + indices2.size());
	// test some index values
	// mesh 1
	EXPECT_EQ(merged.getIndices()[0], indices1[0]);
	EXPECT_EQ(merged.getIndices()[3], indices1[3]);
	// mesh 2 with adjusted indices
	const auto stride = getAttributeElementStride(merged.getAttributes());
	EXPECT_EQ(merged.getIndices()[indices1.size()], indices2[0] + vertices1.size() / stride);
	EXPECT_EQ(merged.getIndices()[indices1.size() + 2], indices2[2] + vertices1.size() / stride);

	// check vertices
	EXPECT_EQ(merged.getVertices().size(), vertices1.size() + vertices2.size());
	// some mesh 1 vertices
	EXPECT_EQ(merged.getVertices()[0], vertices1[0]);
	EXPECT_EQ(merged.getVertices()[stride], vertices1[stride]);
	EXPECT_EQ(merged.getVertices()[2 * stride], vertices1[2 * stride]);
	// some mesh 2 vertices
	EXPECT_EQ(merged.getVertices()[vertices1.size()], vertices2[0]);
	EXPECT_EQ(merged.getVertices()[vertices1.size() + stride], vertices2[stride]);
	EXPECT_EQ(merged.getVertices()[vertices1.size() + 2 * stride], vertices2[2 * stride]);
	EXPECT_NO_THROW(merged.verify());
}

TEST(TestSuite, MergeError)
{
	// incompatible attributes
	std::vector<BinaryMesh> meshes;
	meshes.emplace_back(Normal, std::vector<float>{}, std::vector<uint32_t>{}, std::vector<BinaryMesh::Shape>{});
	meshes.emplace_back(Position, std::vector<float>{}, std::vector<uint32_t>{}, std::vector<BinaryMesh::Shape>{});

	EXPECT_THROW(BinaryMesh::mergeShapes(meshes), std::runtime_error);
}