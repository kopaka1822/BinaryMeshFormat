#include "pch.h"
#include "../include/bmf/generators/ConstantValueGenerator.h"
#include "../include/bmf/generators/FlatNormalGenerator.h"

#define TestSuite GeneratorTest

TEST(TestSuite, FlatNormals)
{
	const std::vector<float> vertices = {
		0.0f, 0.0f, 0.0f, // vertex 1
		1.0f, 0.0f, 0.0f, // vertex 2
		0.0f, 0.0f, 1.0f, // vertex 3
		1.0f, 1.0f, 0.0f, // vertex 4
	};
	const std::vector<uint32_t> indices = {
	0, 2, 1, // triangle 1 (laying on the ground with normal up)
	3, 0, 1, // triangle 2 (standing and facing in positive z)
	};
	const std::vector<BinaryMesh::Shape> shapes = {
		BinaryMesh::Shape{0, 6, 2}, // shape
	};

	BinaryMesh m1(Position, vertices, indices, shapes);

	std::vector<std::unique_ptr<VertexGenerator>> generators;
	generators.emplace_back(new FlatNormalGenerator());

	auto res = m1.changeAttributes(Position | Normal, generators);

	const float normal1[] = { 0.0f, 1.0f, 0.0f };
	const float normal2[] = { 0.0f, 0.0f, 1.0f };

	const std::vector<float> expectedVertices = {
		vertices[0], vertices[1], vertices[2], normal1[0], normal1[1], normal1[2], // vertex 1 (first triangle)
		vertices[0], vertices[1], vertices[2], normal2[0], normal2[1], normal2[2], // vertex 1 (second triangle)
		vertices[3], vertices[4], vertices[5], normal1[0], normal1[1], normal1[2], // vertex 2 (first triangle)
		vertices[3], vertices[4], vertices[5], normal2[0], normal2[1], normal2[2], // vertex 2 (second triangle)
		vertices[6], vertices[7], vertices[8], normal1[0], normal1[1], normal1[2], // vertex 3 (isolated)
		vertices[9], vertices[10], vertices[11], normal2[0], normal2[1], normal2[2], // vertex 4 (isolated)
	};
	const std::vector<uint32_t> expectedIndices = {
		0, 4, 2, // first triangle
		5, 1, 3, // second triangle
	};
	
	EXPECT_EQ(res.getVertices(), expectedVertices);
	EXPECT_EQ(res.getIndices(), expectedIndices);
}