#include "../include/bmf/generators/InterpolatedNormalGenerator.h"
#include "../dependencies/glm/glm/vec3.hpp"
#include "../include/bmf/generators/FlatNormalGenerator.h"
#include "../dependencies/glm/glm/gtc/type_ptr.hpp"

uint32_t bmf::InterpolatedNormalGenerator::getRequiredAttributes() const
{
	return Position;
}

uint32_t bmf::InterpolatedNormalGenerator::getOutputAttribute(uint32_t inAttr) const
{
	return inAttr | Normal;
}

void bmf::InterpolatedNormalGenerator::generate(const std::vector<Triangle>& triangles,
	std::vector<ValueVertex>& outVertices, std::vector<uint32_t>& outIndices) const
{
	// just take the vertex from the first triangle to obtain information
	auto& vertex = triangles.front().vertex[0];

	ValueVertex res(vertex.getAttributes() | Normal);
	// use same position etc.
	vertex.copyAttributesTo(res);
	// overwrite normal
	getInterpolatedNormal(triangles, res.get(Normal));

	outVertices.emplace_back(std::move(res));
}

void bmf::InterpolatedNormalGenerator::getInterpolatedNormal(const std::vector<Triangle>& triangles, float* dstNormal)
{
	auto& normal = *reinterpret_cast<glm::vec3*>(dstNormal);
	normal = glm::vec3(0);
	// sum all flat normals
	for(const auto& tri : triangles)
	{
		glm::vec3 n;
		FlatNormalGenerator::getFlatNormal(tri, glm::value_ptr(n));
		normal += n;
	}
	
	// take the average of all flat normals
	normal /= triangles.size();
	normal = glm::normalize(normal);
}
