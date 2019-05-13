#include "../include/bmf/generators/FlatNormalGenerator.h"
#include "../include/bmf/generators/glm.h"

uint32_t bmf::FlatNormalGenerator::getRequiredAttributes() const
{
	return Position;
}

uint32_t bmf::FlatNormalGenerator::getOutputAttribute(uint32_t inAttr) const
{
	return inAttr | Normal;
}

void bmf::FlatNormalGenerator::generate(const std::vector<Triangle>& triangles,
	std::vector<ValueVertex>& outVertices,
	std::vector<uint32_t>& outIndices) const
{
	// generate flat normal for each triangle
	for (auto& tri : triangles)
	{
		glm::vec3 n;
		getFlatNormal(tri, glm::value_ptr(n));

		// does the vertex already exist?
		bool foundVertex = false;
		for(size_t i = 0, end = outVertices.size(); i < end; ++i)
		{
			auto other = toVec3(outVertices[i].get(Normal));
			auto diff = glm::abs(n - other);
			if (diff.x + diff.y + diff.z > NormalEqualEpsilon)
				continue; // too different

			// almost the same normal => use this vertex
			outIndices.push_back(uint32_t(i));
			foundVertex = true;
			break;
		}

		if (foundVertex) continue;

		// set normal
		ValueVertex newVertex(tri.vertex[0].getAttributes() | Normal);
		tri.vertex[0].copyAttributesTo(newVertex);
		newVertex.set(Normal, glm::value_ptr(n));

		outIndices.push_back(uint32_t(outVertices.size()));
		outVertices.emplace_back(std::move(newVertex));
	}
}

void bmf::FlatNormalGenerator::getFlatNormal(const Triangle& triangle, float* destNormal)
{
	const auto p0 = toVec3(triangle.vertex[0].get(Position));
	const auto p1 = toVec3(triangle.vertex[1].get(Position));
	const auto p2 = toVec3(triangle.vertex[2].get(Position));

	const auto v1 = p1 - p0; // p0p1 vec
	const auto v2 = p2 - p0; // p0p2 vec

	auto& n = *reinterpret_cast<glm::vec3*>(destNormal);
	n = glm::cross(v1, v2);
	n = glm::normalize(n);
}
