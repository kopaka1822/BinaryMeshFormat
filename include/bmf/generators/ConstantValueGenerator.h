#pragma once
#include "../Vertex.h"
#include "../VertexGenerator.h"

namespace bmf
{
	class ConstantValueGenerator final : public SingleVertexGenerator
	{
	public:
		explicit ConstantValueGenerator(ValueVertex value);

		uint32_t getRequiredAttributes() const override;
		uint32_t getOutputAttribute(uint32_t inAttr) const override;
		void generate(Vertex& out) const override;
	private:
		ValueVertex m_value;
	};

	inline ConstantValueGenerator::ConstantValueGenerator(ValueVertex value)
		:
	m_value(value)
	{}

	inline uint32_t ConstantValueGenerator::getRequiredAttributes() const
	{
		return 0;
	}

	inline uint32_t ConstantValueGenerator::getOutputAttribute(uint32_t inAttr) const
	{
		return inAttr | m_value.getAttributes();
	}

	inline void ConstantValueGenerator::generate(Vertex& out) const
	{
		m_value.copyAttributesTo(out);
	}
}
