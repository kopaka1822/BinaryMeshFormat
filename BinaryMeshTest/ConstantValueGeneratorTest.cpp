#include "pch.h"
#include "../include/bmf/generators/ConstantValueGenerator.h"

#define TestSuite GeneratorTest

TEST(TestSuite, ConstantValue)
{
	const std::vector<float> vertices = {
		0.0f, 0.0f, 0.0f, // vertex 1
		1.0f, 0.0f, 1.0f, // vertex 2
		0.0f, 1.0f, 0.0f, // vertex 3
		1.0f, 0.0f, 2.0f, // vertex 4
	};
	const std::vector<uint32_t> indices = {
	0, 1, 2, // triangle 1
	1, 2, 3, // triangle 2
	};
	const std::vector<BinaryMesh::Shape> shapes = {
		BinaryMesh::Shape{0, 6, 2}, // shape
	};

	BinaryMesh m1(Position, vertices, indices, shapes);

	std::vector<std::unique_ptr<VertexGenerator>> generators;
	const float texValue[] = { 2.0f, 3.0f };
	generators.emplace_back(new ConstantValueGenerator(ValueVertex(Texcoord1, texValue)));

	auto res = m1.changeAttributes(Position | Texcoord1, generators);
	// indices should not change
	EXPECT_EQ(res.getIndices(), m1.getIndices());
	// check vertices
	// vertex 1
	EXPECT_EQ(res.getVertices()[0], vertices[0]); // original
	EXPECT_EQ(res.getVertices()[1], vertices[1]); // original
	EXPECT_EQ(res.getVertices()[2], vertices[2]); // original
	EXPECT_EQ(res.getVertices()[3], texValue[0]); // new
	EXPECT_EQ(res.getVertices()[4], texValue[1]); // new
	// vertex 2
	EXPECT_EQ(res.getVertices()[5], vertices[3]); // original
	EXPECT_EQ(res.getVertices()[6], vertices[4]); // original
	EXPECT_EQ(res.getVertices()[7], vertices[5]); // original
	EXPECT_EQ(res.getVertices()[8], texValue[0]); // new
	EXPECT_EQ(res.getVertices()[9], texValue[1]); // new
}