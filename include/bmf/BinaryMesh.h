#pragma once
#include <cstdint>
#include <vector>
#include <fstream>
#include <algorithm>
#include "Attributes.h"
#include "VertexGenerator.h"

namespace bmf
{
	class BinaryMesh
	{
	public:
		struct Shape
		{
			// offset for the first index in the index buffer
			uint32_t indexOffset;
			// number of used indices
			uint32_t indexCount;
			// material if for this shape
			uint32_t materialId;
		};

#pragma region Getter
		/// \brief returns bitmask of all used attributes (see Attributes enum)
		uint32_t getAttributes() const;
		const std::vector<float>& getVertices() const;
		const std::vector<uint32_t>& getIndices() const;
		const std::vector<Shape>& getShapes() const;
		/// \brief returns byte offset to the attribute
		uint32_t getAttributeByteOffset(Attributes a) const;
		/// \brief returns byte size of one vertex
		uint32_t getAttributeByteStride() const;
#pragma endregion
#pragma region FileIO
		static BinaryMesh loadFromFile(const std::string& filename);
		void saveToFile(const std::string& filename);
#pragma endregion
#pragma region Grouping
		std::vector<BinaryMesh> splitShapes() const;
		static BinaryMesh mergeShapes(const std::vector<BinaryMesh>& meshes);
#pragma endregion
#pragma region Generating
		BinaryMesh changeAttributes(uint32_t newAttributes,
		                            const std::vector<std::unique_ptr<VertexGenerator>>& generators) const;
#pragma endregion
#pragma region Ctor
		BinaryMesh(uint32_t attributes, std::vector<float> vertices, std::vector<uint32_t> indices,
		           std::vector<Shape> shapes);
		BinaryMesh() = default;
		~BinaryMesh() = default;
		BinaryMesh(const BinaryMesh&) = default;
		BinaryMesh& operator=(const BinaryMesh&) = default;
		BinaryMesh(BinaryMesh&&) noexcept = default;
		BinaryMesh& operator=(BinaryMesh&&) noexcept = default;
#pragma endregion
	private:
		BinaryMesh useVertexGenerator(const SingleVertexGenerator& svgen) const;
		BinaryMesh useVertexGenerator(const MultiVertexGenerator& mvgen) const;

		std::vector<float> m_vertices;
		std::vector<uint32_t> m_indices;
		std::vector<Shape> m_shapes;

		uint32_t m_attributes = 0;

		static constexpr uint32_t s_version = 0;
	};

#pragma region Getter
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

	inline uint32_t BinaryMesh::getAttributeByteOffset(Attributes a) const
	{
		return bmf::getAttributeByteOffset(m_attributes, a);
	}

	inline uint32_t BinaryMesh::getAttributeByteStride() const
	{
		return bmf::getAttributeByteOffset(m_attributes, Attributes::SIZE);
	}
#pragma endregion
#pragma region FileIO
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
		BinaryMesh m;
		m.m_attributes = ui32;

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
#pragma endregion
#pragma region Grouping
	inline std::vector<BinaryMesh> BinaryMesh::splitShapes() const
	{
		std::vector<BinaryMesh> meshes;
		meshes.resize(m_shapes.size());
		const auto attributeCount = getAttributeElementStride(m_attributes);

		for (size_t i = 0; i < meshes.size(); ++i)
		{
			const auto& src = m_shapes[i];
			auto& dst = meshes[i];
			// keep attributes
			dst.m_attributes = m_attributes;
			// only one shape
			dst.m_shapes.push_back(src);
			dst.m_shapes[0].indexOffset = 0; // starts immediately

			// vertex range
			size_t firstVertexIndex = m_vertices.size() - 1;
			size_t lastVertexIndex = 0;

			dst.m_indices.resize(src.indexCount);
			// copy indices and remember vertex offsets
			std::transform(
				m_indices.begin() + src.indexOffset,
				m_indices.begin() + src.indexOffset + src.indexCount,
				dst.m_indices.begin(), [&firstVertexIndex, &lastVertexIndex](auto idx)
				{
					// remember index of first and last used vertex
					firstVertexIndex = std::min(idx, firstVertexIndex);
					lastVertexIndex = std::max(idx, lastVertexIndex);
					return idx;
				});

			// subtract firstVertexIndex (first vertex should start at 0)
			for (auto& idx : dst.m_indices)
				idx -= firstVertexIndex;

			// change to one after last
			++lastVertexIndex;

			// copy used vertices
			dst.m_vertices.resize((lastVertexIndex - firstVertexIndex) * attributeCount);
			std::copy(
				m_vertices.begin() + (firstVertexIndex * attributeCount),
				m_vertices.begin() + (lastVertexIndex * attributeCount),
				dst.m_vertices.begin());
		}

		return meshes;
	}

	inline BinaryMesh BinaryMesh::mergeShapes(const std::vector<BinaryMesh>& meshes)
	{
		if (meshes.empty()) return BinaryMesh();

		BinaryMesh m;
		m.m_attributes = meshes.front().m_attributes;
		const auto attributeCount = getAttributeElementStride(meshes.front().m_attributes);
		size_t m_totalVertices = 0;
		size_t m_totalIndices = 0;
		size_t m_totalShapes = 0;

		// acquire total vertex/index count
		for (const auto& src : meshes)
		{
			if (src.m_attributes != m.m_attributes)
				throw std::runtime_error("BinaryMesh::mergeShapes attributes of all meshes must be the same");

			m_totalVertices += src.m_vertices.size();
			m_totalIndices += src.m_indices.size();
			m_totalShapes += src.m_shapes.size();
		}

		// resize buffers
		m.m_shapes.resize(m_totalShapes);
		m.m_indices.resize(m_totalIndices);
		m.m_vertices.resize(m_totalVertices);

		auto curShape = m.m_shapes.begin();
		auto curIndex = m.m_indices.begin();
		auto curVertex = m.m_vertices.begin();

		// copy data
		for (const auto& src : meshes)
		{
			// start index of the new vertices
			auto vertexOffset = (curVertex - m.m_vertices.begin()) / attributeCount;
			// additional index offset for shapes
			auto indexOffset = curIndex - m.m_indices.begin();

			curVertex = std::copy(src.m_vertices.begin(), src.m_vertices.end(), curVertex);
			curIndex = std::transform(src.m_indices.begin(), src.m_indices.end(), curIndex, [vertexOffset](auto index)
			{
				return index + vertexOffset;
			});
			curShape = std::transform(src.m_shapes.begin(), src.m_shapes.end(), curShape, [indexOffset](Shape s)
			{
				s.indexOffset += indexOffset;
				return s;
			});
		}

		return m;
	}
#pragma endregion
#pragma region Generation
	inline BinaryMesh BinaryMesh::changeAttributes(uint32_t newAttributes,
	                                               const std::vector<std::unique_ptr<VertexGenerator>>& generators) const
	{
		// simple copy
		if (newAttributes == m_attributes) return *this;
		
		// everything generated?
		if((newAttributes & m_attributes) == newAttributes)
		{
			// remove unnecessary attributes
			const auto oldVertexStride = getAttributeElementStride(m_attributes);
			const auto vertexCount = m_vertices.size();
			const auto newVertexStride = getAttributeElementStride(newAttributes);
			std::vector<float> newVertices(newVertexStride * vertexCount);
			
			for(size_t i = 0; i < vertexCount; ++i)
			{
				const RefVertex src(m_attributes, const_cast<float*>(&m_vertices[i * oldVertexStride]));
				RefVertex dst(m_attributes, &newVertices[i * newVertexStride]);
				// change attributes to match destination
				src.copyAttributesTo(dst);
			}

			return BinaryMesh(newAttributes, newVertices, m_indices, m_shapes);
		}

		// find first promising generator
		auto gen = generators.begin();
		for (const auto end = generators.end(); gen != end; ++gen)
		{
			// can this generator be used?
			if(((*gen)->getRequiredAttributes() & m_attributes) != (*gen)->getRequiredAttributes())
				continue;

			// will this generator produce something useful?
			auto outAttribs = (*gen)->getOutputAttribute(m_attributes);
			if ((newAttributes & outAttribs) > (newAttributes & m_attributes))
				break;
		}

		if (gen == generators.end())
			throw std::runtime_error("BinaryMesh::changeAttributes no matching generator found to convert to new attributes");

		// use the generator to generate a new mesh
		const auto svgen = dynamic_cast<SingleVertexGenerator*>((*gen).get());
		if(svgen != nullptr)
			return useVertexGenerator(*svgen);

		const auto mvgen = dynamic_cast<MultiVertexGenerator*>((*gen).get());
		if (mvgen != nullptr)
			return useVertexGenerator(*mvgen);

		throw std::runtime_error("BinaryMesh::changeAttributes incompatible vertex generator type");
	}
#pragma endregion
#pragma region Ctor
	inline BinaryMesh::BinaryMesh(uint32_t attributes, std::vector<float> vertices, std::vector<uint32_t> indices,
	                              std::vector<Shape> shapes)
		:
		m_vertices(move(vertices)),
		m_indices(move(indices)),
		m_shapes(move(shapes)),
		m_attributes(attributes)
	{
	}

	inline BinaryMesh BinaryMesh::useVertexGenerator(const SingleVertexGenerator& svgen) const
	{
		const auto newAttributes = svgen.getOutputAttribute(m_attributes);
		const auto oldVertexStride = getAttributeElementStride(m_attributes);
		const auto vertexCount = m_vertices.size();
		const auto newVertexStride = getAttributeElementStride(newAttributes);
		std::vector<float> newVertices(newVertexStride * vertexCount);

		// convert all vertices
		for(size_t i = 0; i < vertexCount; ++i)
		{
			const RefVertex src(m_attributes, const_cast<float*>(&m_vertices[i * oldVertexStride]));
			RefVertex dst(m_attributes, &newVertices[i * newVertexStride]);

			src.copyAttributesTo(dst);
			// apply generator
			svgen.generate(dst);
		}

		return BinaryMesh(newAttributes, newVertices, m_indices, m_shapes);
	}

	inline BinaryMesh BinaryMesh::useVertexGenerator(const MultiVertexGenerator& mvgen) const
	{
		const auto newAttributes = mvgen.getOutputAttribute(m_attributes);
		const auto oldVertexStride = getAttributeElementStride(m_attributes);
		const auto vertexCount = m_vertices.size();
		const auto newVertexStride = getAttributeElementStride(newAttributes);

		std::vector<float> newVertices;
		newVertices.reserve(newVertexStride * vertexCount);
		std::vector<uint32_t> newIndices(m_indices);

		std::vector<IndexTriangle> triangleIndices;
		std::vector<Triangle> triangles;
		std::vector<ValueVertex> valueVertices;
		std::vector<uint32_t>  outIndices;
		valueVertices.reserve(16);
		triangleIndices.reserve(16);
		triangles.reserve(16);
		outIndices.reserve(16);

		for(size_t vertIdx = 0; vertIdx < vertexCount; ++vertIdx)
		{
			triangleIndices.resize(0);
			triangles.resize(0);
			valueVertices.resize(0);
			outIndices.resize(0);

			// find all triangles that reference this vertex
			for(size_t idxIdx = 0, idxSize = m_indices.size(); idxIdx < idxSize; idxIdx += 3)
			{
				if(m_indices[idxIdx] != vertIdx && m_indices[idxIdx + 1] != vertIdx && m_indices[idxIdx + 1] != vertIdx)
					continue;

				// this triangle matches
				triangleIndices.emplace_back();
				auto& itri = triangleIndices.back();
				itri.index[0] = &newIndices[idxIdx];
				itri.index[1] = &newIndices[idxIdx + 1];
				itri.index[2] = &newIndices[idxIdx + 2];

				// rotate triangle if vertex index is not first index
				while (*itri.index[0] != vertIdx) itri.rotateLeft();

				// add triangle representation
				triangles.emplace_back();
				auto& tri = triangles.back();
				tri.vertex[0] = RefVertex(m_attributes, const_cast<float*>(&m_vertices[*itri.index[0] * oldVertexStride]));
				tri.vertex[1] = RefVertex(m_attributes, const_cast<float*>(&m_vertices[*itri.index[1] * oldVertexStride]));
				tri.vertex[2] = RefVertex(m_attributes, const_cast<float*>(&m_vertices[*itri.index[2] * oldVertexStride]));
			}

			// use generator to generate new vertices
			outIndices.resize(triangles.size());
			mvgen.generate(triangles, valueVertices, outIndices);

			assert(valueVertices.size());

			// add vertices
			const auto startIdx = newVertices.size();
			const auto startElementIdx = startIdx / oldVertexStride;

			newVertices.resize(newVertices.size() + valueVertices.size() * newVertexStride);
			// copy vertices
			for(size_t i = 0; i < valueVertices.size(); ++i)
			{
				std::copy(
					valueVertices[i].data(),
					valueVertices[i].data() + newVertexStride,
					newVertices.data() + startIdx + i * newVertexStride);
			}

			// adjust indices
			assert(outIndices.size() == triangles.size());
			assert(triangles.size() == triangleIndices.size());
			for(size_t i = 0; i < triangleIndices.size(); ++i)
			{
				*triangleIndices[i].index[0] = startElementIdx + outIndices[i];
			}
		}

		return BinaryMesh(newAttributes, newVertices, newIndices, m_shapes);
	}
#pragma endregion
}