#include "pch.h"

#define TestSuite GeneratorTest

TEST(TestSuite, FlatNormalsDifferentNormals) // two triangles with different normals
{
	using BinaryMesh = BinaryMesh32;

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
	const std::vector<Shape> shapes = {
		Shape{0, 6, 0, 4, /*0, 1,*/ 2}, // shape
	};

	BinaryMesh m1(Position, vertices, indices, shapes);//, getIdentityVec(1));
	m1.generateBoundingBoxes();

	std::vector<std::unique_ptr<VertexGenerator>> generators;
	generators.emplace_back(new FlatNormalGenerator());

	BinaryMesh res(m1);
	res.changeAttributes(Position | Normal, generators);

	const float normal1[] = { 0.0f, 1.0f, 0.0f };
	const float normal2[] = { 0.0f, 0.0f, 1.0f };

	const std::vector<float> expectedVertices = {
		vertices[0], vertices[1], vertices[2], normal1[0], normal1[1], normal1[2], // vertex 0 (first triangle)
		vertices[0], vertices[1], vertices[2], normal2[0], normal2[1], normal2[2], // vertex 0 (second triangle)
		vertices[3], vertices[4], vertices[5], normal1[0], normal1[1], normal1[2], // vertex 1 (first triangle)
		vertices[3], vertices[4], vertices[5], normal2[0], normal2[1], normal2[2], // vertex 1 (second triangle)
		vertices[6], vertices[7], vertices[8], normal1[0], normal1[1], normal1[2], // vertex 2 (isolated)
		vertices[9], vertices[10], vertices[11], normal2[0], normal2[1], normal2[2], // vertex 3 (isolated)
	};
	const std::vector<uint32_t> expectedIndices = {
		0, 4, 2, // first triangle
		5, 1, 3, // second triangle
	};
	
	EXPECT_EQ(res.getVertices(), expectedVertices);
	EXPECT_EQ(res.getIndices(), expectedIndices);

	EXPECT_NO_THROW(res.verify());
}

TEST(TestSuite, FlatNormalsSameNormals) // two triangles with the same normals
{
	using BinaryMesh = BinaryMesh32;

	const std::vector<float> vertices = {
		0.0f, 0.0f, 0.0f, // vertex 0
		1.0f, 0.0f, 0.0f, // vertex 1
		1.0f, 0.0f, 1.0f, // vertex 2
		0.0f, 0.0f, 1.0f, // vertex 3
	};
	const std::vector<uint32_t> indices = {
		0, 1, 2, // triangle 1 (laying on the ground with normal down)
		0, 2, 3, // triangle 2 (laying on the ground with normal down)
	};
	const std::vector<Shape> shapes = {
		Shape{0, 6, 0, 4, /*0, 1,*/ 2}, // shape
	};

	BinaryMesh m1(Position, vertices, indices, shapes);// , getIdentityVec(1));
	m1.generateBoundingBoxes();

	std::vector<std::unique_ptr<VertexGenerator>> generators;
	generators.emplace_back(new FlatNormalGenerator());

	BinaryMesh res(m1);
	res.changeAttributes(Position | Normal, generators);

	const float normal[] = { 0.0f, -1.0f, 0.0f };
	
	const std::vector<float> expectedVertices = {
		vertices[0], vertices[1], vertices[2], normal[0], normal[1], normal[2], // vertex 0
		vertices[3], vertices[4], vertices[5], normal[0], normal[1], normal[2], // vertex 1
		vertices[6], vertices[7], vertices[8], normal[0], normal[1], normal[2], // vertex 2
		vertices[9], vertices[10], vertices[11], normal[0], normal[1], normal[2], // vertex 3
	};

	EXPECT_EQ(res.getVertices(), expectedVertices);
	// indices should remain
	EXPECT_EQ(res.getIndices(), indices);

	EXPECT_NO_THROW(res.verify());
}