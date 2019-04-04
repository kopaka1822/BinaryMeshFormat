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

		/// \brief will be called for each vertex
		/// \param triangles all triangles where the current vertex is used in. 
		/// The current vertex is always the first vertex in the triangle.
		/// \param outVertices the converted vertices that should be used for the new mesh. 
		/// The vector is empty (but enough space was reserved).
		/// \param outIndices indicates which vertices from the outVertices belong to which mesh. 
		/// The vector is empty (but enough space was reserved).
		/// outIndices[a] = b means: The triangle at position a uses the vertex at position b in outVertices.
		virtual void generate(
			const std::vector<Triangle>& triangles, 
			std::vector<ValueVertex>& outVertices, 
			std::vector<uint32_t>& outIndices) const = 0;
	};
}
