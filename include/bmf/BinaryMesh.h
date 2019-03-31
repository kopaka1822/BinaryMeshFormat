#pragma once
#include <cstdint>
#include <vector>
#include <fstream>

namespace bmf
{
	class BinaryMesh
	{
	public:
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
			SIZE // reserved
		};
		
		struct Shape
		{
			// offset for the first index in the index buffer
			uint32_t indexOffset;
			// number of used indices
			uint32_t indexCount;
			// material if for this shape
			uint32_t materialId;
		};

		/// \brief returns bitmask of all used attributes (see Attributes enum)
		uint32_t getAttributes() const;
		const std::vector<float>& getVertices() const;
		const std::vector<uint32_t>& getIndices() const;
		const std::vector<Shape>& getShapes() const;
		/// \brief returns byte offset to the attribute
		uint32_t getAttributeOffset(Attributes a) const;
		/// \brief returns byte size of the attribute
		static constexpr uint32_t getAttributeSize(Attributes a);
		/// \brief returns byte size of one vertex
		uint32_t getAttributeStride() const;

		static BinaryMesh loadFromFile(const std::string& filename);
		void saveToFile(const std::string& filename);
		/// \brief creates a new BinaryMesh with the new attributes by using the vertices of this mesh
		//BinaryMesh copy(Attributes newAttributes);
		

		BinaryMesh(uint32_t attributes);

		BinaryMesh() = default;
		~BinaryMesh() = default;
		BinaryMesh(const BinaryMesh&) = default;
		BinaryMesh& operator=(const BinaryMesh&) = default;
		BinaryMesh(BinaryMesh&&) noexcept = default;
		BinaryMesh& operator=(BinaryMesh&&) noexcept = default;
	private:
		std::vector<float> m_vertices;
		std::vector<uint32_t> m_indices;
		std::vector<Shape> m_shapes;

		uint32_t m_attributes = 0;

		static constexpr uint32_t s_version = 0;
	};

	inline uint32_t BinaryMesh::getAttributes() const
	{
		return m_attributes;
	}

	inline const std::vector<float>& BinaryMesh::getVertices() const
	{
		return m_vertices;
	}

	inline const std::vector<uint32_t>& BinaryMesh::getIndices() const
	{
		return m_indices;
	}

	inline const std::vector<BinaryMesh::Shape>& BinaryMesh::getShapes() const
	{
		return m_shapes;
	}

	inline uint32_t BinaryMesh::getAttributeOffset(Attributes a) const
	{
		uint32_t offset = 0;
		// accumulate attribute stride of all used attributes before the searched attribute
		for (uint32_t cur = 1; cur < uint32_t(a); cur = cur << 1)
		{
			if (m_attributes & cur)
				offset += getAttributeSize(Attributes(a));
		}
		return offset;
	}

	inline uint32_t BinaryMesh::getAttributeSize(Attributes a)
	{
		switch (a) 
		{ 
			case Position:	return 3 * sizeof(float);
			case Normal:	return 3 * sizeof(float);
			case Texcoord0: return 2 * sizeof(float);
			case Tangent:	return 3 * sizeof(float);
			case BiTangent: return 3 * sizeof(float);
			case Color:		return 4 * sizeof(float);
			case Texcoord1: return 2 * sizeof(float);
			case Texcoord2: return 2 * sizeof(float);
			case Texcoord3: return 2 * sizeof(float);
			default: return 0;
		}
	}

	inline BinaryMesh::BinaryMesh(uint32_t attributes):
		m_attributes(attributes)
	{
	}

	inline uint32_t BinaryMesh::getAttributeStride() const
	{
		return getAttributeOffset(Attributes::SIZE);
	}

	inline BinaryMesh BinaryMesh::loadFromFile(const std::string& filename)
	{
		std::fstream f(filename, std::ios::in | std::ios::binary);

		// check file signature
		char sig[3];
		f >> sig[0] >> sig[1] >> sig[2];

		if (memcmp(sig, "BMF", 3) != 0)
			throw std::runtime_error("invalid file signature");

		uint32_t ui32;
		f >> ui32; // version number
		if (ui32 != s_version)
			throw std::runtime_error("invalid file version");

		f >> ui32; // attributes

		// create mesh with attributes
		BinaryMesh m(ui32);

		// read vertices
		f >> ui32; // num vertices
		m.m_vertices.resize(ui32);
		// read vertex data
		f.read(reinterpret_cast<char*>(m.m_vertices.data()), ui32 * sizeof(m.m_vertices[0]));

		// read indices
		f >> ui32; // num indices
		m.m_indices.resize(ui32);
		// read index data
		f.read(reinterpret_cast<char*>(m.m_indices.data()), ui32 * sizeof(m.m_indices[0]));

		// read shapes
		f >> ui32; // num shapes
		m.m_shapes.resize(ui32);
		// read shape data
		f.read(reinterpret_cast<char*>(m.m_shapes.data()), ui32 * sizeof(m.m_shapes[0]));

		// check file end signature
		f >> sig[0] >> sig[1] >> sig[2];
		if (memcmp(sig, "EOF", 3) != 0)
			throw std::runtime_error("invalid end of file signature");

		f.close();

		return m;
	}

	inline void BinaryMesh::saveToFile(const std::string& filename)
	{
		std::fstream f(filename, std::ios::out | std::ios::binary);

		// write signature
		f << 'B' << 'M' << 'F';

		// write version number
		f << s_version;

		// write attributes
		f << m_attributes;

		// write vertices
		f << uint32_t(m_vertices.size());
		f.write(reinterpret_cast<const char*>(m_vertices.data()), m_vertices.size() * sizeof(m_vertices[0]));

		// write indices
		f << uint32_t(m_indices.size());
		f.write(reinterpret_cast<const char*>(m_indices.data()), m_indices.size() * sizeof(m_indices[0]));

		// write shapes
		f << uint32_t(m_shapes.size());
		f.write(reinterpret_cast<const char*>(m_shapes.data()), m_shapes.size() * sizeof(m_shapes[0]));

		// write end of file signature
		f << 'E' << 'O' << 'F';

		f.close();
	}
}
