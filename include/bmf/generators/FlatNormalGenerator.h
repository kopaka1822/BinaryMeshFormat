#pragma once
#include "../VertexGenerator.h"

namespace bmf
{
	class FlatNormalGenerator final : MultiVertexGenerator
	{
		static constexpr float NormalEqualEpsilon = 0.0001f;
	public:
		uint32_t getRequiredAttributes() const override;
		uint32_t getOutputAttribute(uint32_t inAttr) const override;
		void generate(
			const std::vector<Triangle>& triangles, 
			std::vector<ValueVertex>& outVertices,
		    std::vector<uint32_t>& outIndices) const override;
	};
}
