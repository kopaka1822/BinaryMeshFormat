#include "pch.h"

#define TestSuite GeneratorTest

TEST(TestSuite, InterpolatedNormals)
{
	const std::vector<float> vertices = {
		0.0f, 0.0f, 0.0f, // vertex 0
		1.0f, 0.0f, 0.0f, // vertex 1
		0.0f, 0.0f, 1.0f, // vertex 2
		1.0f, 1.0f, 0.0f, // vertex 3
	};
	const std::vector<uint32_t> indices = {
		0, 2, 1, // triangle 1 (laying on the ground with normal up)
		3, 0, 1, // triangle 2 (standing and facing in positive z)
	};
	const std::vector<BinaryMesh::Shape> shapes = {
		BinaryMesh::Shape{0, 6, 0, 4, 0, 1, 2}, // shape
	};

	BinaryMesh m1(Position, vertices, indices, shapes, getIdentityVec(1));

	std::vector<std::unique_ptr<VertexGenerator>> generators;
	generators.emplace_back(new InterpolatedNormalGenerator());

	BinaryMesh res(m1);
	res.changeAttributes(Position | Normal, generators);

	const float normal1[] = { 0.0f, 1.0f, 0.0f };
	const float normal2[] = { 0.0f, 0.0f, 1.0f };
	const float normalInt[] = {0.0f, 1.0f / sqrt(2.0f), 1.0f / sqrt(2.0f)};

	const std::vector<float> expectedVertices = {
		vertices[0], vertices[1], vertices[2], normalInt[0], normalInt[1], normalInt[2], // vertex 0 (shared edge)
		vertices[3], vertices[4], vertices[5], normalInt[0], normalInt[1], normalInt[2], // vertex 1 (shared edge)
		vertices[6], vertices[7], vertices[8], normal1[0], normal1[1], normal1[2], // vertex 2 (isolated ground)
		vertices[9], vertices[10], vertices[11], normal2[0], normal2[1], normal2[2], // vertex (isolated standing)
	};

	EXPECT_EQ(res.getVertices(), expectedVertices);
	// indices remain
	EXPECT_EQ(res.getIndices(), m1.getIndices());

	EXPECT_NO_THROW(res.verify());
}