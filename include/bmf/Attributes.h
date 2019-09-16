#pragma once
#include <cstdint>

namespace bmf
{
	enum Attributes
	{
		Position = 1,		// float3
		Normal = 1 << 1,	// float3
		Texcoord0 = 1 << 2,	// float2
		Tangent = 1 << 3,	// float3
		BiTangent = 1 << 4, // float3
		Color = 1 << 5,		// float4
		Texcoord1 = 1 << 6,	// float2
		Texcoord2 = 1 << 7,	// float2
		Texcoord3 = 1 << 8, // float2
		Width = 1 << 9,     // float
		Height = 1 << 10,   // float
		Depth = 1 << 11,    // float
		SIZE // reserved
	};

	/// \brief returns number of floats that is used for this attribute
	constexpr uint32_t getAttributeElementCount(Attributes a) noexcept
	{
		switch (a)
		{
		case Position:	return 3;
		case Normal:	return 3;
		case Texcoord0: return 2;
		case Tangent:	return 3;
		case BiTangent: return 3;
		case Color:		return 4;
		case Texcoord1: return 2;
		case Texcoord2: return 2;
		case Texcoord3: return 2;
		case Width:		return 1;
		case Height:    return 1;
		case Depth:     return 1;
		default: return 0;
		}
	}

	/// \brief returns byte size of the attribute
	constexpr uint32_t getAttributeByteCount(Attributes a) noexcept
	{
		return getAttributeElementCount(a) * sizeof(float);
	}

	/// \brief returns number of floats before this attribute
	constexpr uint32_t getAttributeElementOffset(uint32_t all, Attributes a) noexcept
	{
		uint32_t offset = 0;
		// accumulate attribute stride of all used attributes before the searched attribute
		for (uint32_t cur = 1; cur < uint32_t(a); cur = cur << 1)
		{
			if (all & cur)
				offset += getAttributeElementCount(Attributes(cur));
		}
		return offset;
	}

	/// \brief returns byte offset to the attribute
	constexpr uint32_t getAttributeByteOffset(uint32_t all, Attributes a) noexcept
	{
		return getAttributeElementOffset(all, a) * sizeof(float);
	}

	/// \brief return number of floats of one vertex
	constexpr uint32_t getAttributeElementStride(uint32_t attributes) noexcept
	{
		return getAttributeElementOffset(attributes, Attributes::SIZE);
	}

	/// \brief returns byte size of one vertex
	constexpr uint32_t getAttributeByteStride(uint32_t attributes) noexcept
	{
		return getAttributeElementStride(attributes) * sizeof(float);
	}
}