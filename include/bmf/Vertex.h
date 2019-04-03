#pragma once
#include <vector>
#include "Attributes.h"
#include <cassert>
#include "VertexCache.h"

namespace bmf
{
	class Vertex
	{
	public:
		virtual ~Vertex() = default;
		//VertexBase(uint32_t attributes, const float* data);
		constexpr Vertex(uint32_t attributes) noexcept;
		Vertex() noexcept = default;
		float* get(Attributes attr);
		const float* get(Attributes attr) const;
		void copyAttributesTo(Vertex& outVertex) const;
		virtual float* data() = 0;
		const float* data() const;
	private:
		uint32_t m_attributes;
	};

	class ValueVertex final : public Vertex
	{
	public:
		ValueVertex(uint32_t attributes);
		ValueVertex(uint32_t attributes, const float* data);
		~ValueVertex();
		float* data() override;
	private:
		std::vector<float> m_member;
	};

	class RefVertex final : public Vertex
	{
	public:
		constexpr RefVertex(uint32_t attributes, float* data);
		RefVertex() noexcept = default;
		float* data() override;
	private:
		float* m_data;
	};

	inline Vertex::Vertex(uint32_t attributes)
		:
	m_attributes(attributes)
	{}

	inline float* Vertex::get(Attributes attr)
	{
		// vertex should contain attribute
		assert((m_attributes & attr) != 0);
		const auto off = getAttributeElementOffset(m_attributes, attr);
		return data() + off;
	}

	inline const float* Vertex::get(Attributes attr) const
	{
		return const_cast<Vertex*>(this)->get(attr);
	}

	inline void Vertex::copyAttributesTo(Vertex& outVertex) const
	{
		auto newAttributes = outVertex.m_attributes;

		// keep old attributes
		const auto commonAttribs = m_attributes & newAttributes;
		for(uint32_t a = 1; a < uint32_t(Attributes::SIZE); a = a << 1)
		{
			// copy if both have the attribute
			if(a & commonAttribs)
			{
				auto src = get(Attributes(a));
				std::copy(src, 
					src + getAttributeElementCount(Attributes(a)), 
					outVertex.get(Attributes(a)));
			}
		}
	}

	inline const float* Vertex::data() const
	{
		return const_cast<Vertex*>(this)->data();
	}

	inline ValueVertex::ValueVertex(uint32_t attributes)
		:
	Vertex(attributes),
	m_member(VertexCache::get())
	{
		m_member.resize(getAttributeElementStride(attributes));
	}

	inline ValueVertex::ValueVertex(uint32_t attributes, const float* data)
		:
	Vertex(attributes),
	m_member(VertexCache::get())
	{
		m_member.assign(data, data + getAttributeElementStride(attributes));
	}

	inline ValueVertex::~ValueVertex()
	{
		VertexCache::store(m_member);
	}

	inline float* ValueVertex::data()
	{
		return m_member.data();
	}

	inline RefVertex::RefVertex(uint32_t attributes, float* data)
		:
	Vertex(attributes),
	m_data(data)
	{}

	inline float* RefVertex::data()
	{
		return m_data;
	}
}
