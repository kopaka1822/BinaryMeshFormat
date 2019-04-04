#pragma once
#include "../VertexGenerator.h"

namespace bmf
{
	class InterpolatedNormalGenerator : public MultiVertexGenerator
	{
	public:
		uint32_t getRequiredAttributes() const override;
		uint32_t getOutputAttribute(uint32_t inAttr) const override;
		void generate(
			const std::vector<Triangle>& triangles, 
			std::vector<ValueVertex>& outVertices,
			std::vector<uint32_t>& outIndices) const override;
		static void getInterpolatedNormal(const std::vector<Triangle>& triangles, float* dstNormal);
	};
}
