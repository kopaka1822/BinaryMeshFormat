#pragma once
#include <cstdint>
#include "Vertex.h"
#include "Triangle.h"

namespace bmf
{
	class VertexGenerator
	{
	public:
		virtual ~VertexGenerator() = default;
		virtual uint32_t getRequiredAttributes() const = 0;
		virtual uint32_t getOutputAttribute(uint32_t inAttr) const = 0;
	};

	class SingleVertexGenerator : public VertexGenerator
	{
	public:
		virtual void generate(Vertex& out) const = 0;
	};

	class MultiVertexGenerator : public VertexGenerator
	{
	public:
		virtual void generate(
			const std::vector<Triangle>& triangles, 
			std::vector<ValueVertex>& outVertices, 
			std::vector<uint32_t>& outIndices) const = 0;
	};
}
