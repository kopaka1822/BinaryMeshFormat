#pragma once
#include <cstdint>
#include <vector>
#include <fstream>
#include <algorithm>
#include "Attributes.h"
#include "VertexGenerator.h"
#include <unordered_map>
#include "BoundingBox.h"
#include <string>

#ifdef BMF_GENERATORS
#include "generators/ConstantValueGenerator.h"
#include "generators/FlatNormalGenerator.h"
#include "generators/InterpolatedNormalGenerator.h"
#endif

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
			// offset that should be added to each index before reading from the vertex buffer
			uint32_t vertexOffset;
			// number of used vertices
			uint32_t vertexCount;

			// material if for this shape
			uint32_t materialId;

			// bounding box for this shape
			//BoundingBox bbox;
		};

#pragma region Getter
		/// \brief returns bitmask of all used attributes (see Attributes enum)
		uint32_t getAttributes() const;
		std::vector<float>& getVertices();
		const std::vector<float>& getVertices() const;
		std::vector<uint32_t>& getIndices();
		const std::vector<uint32_t>& getIndices() const;
		std::vector<Shape>& getShapes();
		const std::vector<Shape>& getShapes() const;
		/// \brief returns byte offset to the attribute
		uint32_t getAttributeByteOffset(Attributes a) const;
		/// \brief returns byte size of one vertex
		uint32_t getAttributeByteStride() const;
		uint32_t getNumVertices() const;
		/// \brief checks if the mesh has a valid amount of indices/vertices
		/// and no index that goes beyond the buffer.
		/// \throw std::runtime error if something is wrong
		void verify() const;
#pragma endregion
#pragma region FileIO
		static BinaryMesh loadFromFile(const std::string& filename);
		void saveToFile(const std::string& filename) const;
#pragma endregion
#pragma region Grouping
		std::vector<BinaryMesh> splitShapes() const;
		static BinaryMesh mergeShapes(const std::vector<BinaryMesh>& meshes);
#pragma endregion
#pragma region Generating
		void changeAttributes(uint32_t newAttributes,
		                            const std::vector<std::unique_ptr<VertexGenerator>>& generators);
		void removeDuplicateVertices();
		void removeUnusedVertices();
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
#pragma region Generating
		// generator helpers
		void useVertexGenerator(const SingleVertexGenerator& svgen);
		void useVertexGenerator(const MultiVertexGenerator& mvgen);
#pragma endregion 
#pragma region FileIO
		// fstream helpers
		template<class T>
		static void write(std::fstream& stream, const T& value);
		template<class T>
		static void write(std::fstream& stream, const std::vector<T>& vector);
		template<class T>
		static T read(std::fstream& stream);
		template<class T>
		static std::vector<T> read(std::fstream& stream, size_t count);
#pragma endregion 
		void expectSingleShape(const std::string& operation);

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

	inline std::vector<float>& BinaryMesh::getVertices()
	{
		return m_vertices;
	}

	inline const std::vector<float>& BinaryMesh::getVertices() const
	{
		return m_vertices;
	}

	inline std::vector<uint32_t>& BinaryMesh::getIndices()
	{
		return m_indices;
	}

	inline const std::vector<uint32_t>& BinaryMesh::getIndices() const
	{
		return m_indices;
	}

	inline std::vector<BinaryMesh::Shape>& BinaryMesh::getShapes()
	{
		return m_shapes;
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

	inline uint32_t BinaryMesh::getNumVertices() const
	{
		const auto stride = getAttributeElementStride(m_attributes);
		return uint32_t(m_vertices.size() / stride);
	}

	inline void BinaryMesh::verify() const
	{
		// basic tests
		if (m_attributes == 0) 
			throw std::runtime_error("no attributes provided");
		if (m_indices.size() % 3 != 0) 
			throw std::runtime_error("indices are not a multiple of 3");
		const auto stride = getAttributeElementStride(m_attributes);
		if (m_vertices.size() % stride != 0) 
			throw std::runtime_error("vertices are not a multiple of the underlying attribute element stride");

		// are attribute flags valid
		const uint32_t lastAttributeValue = Attributes::SIZE - 1;
		if ((m_attributes & ~((lastAttributeValue << 1) - 1)) != 0)
			throw std::runtime_error("non existent attribute flags were set");

		// shape ranges
		const auto numIndices = m_indices.size();
		const auto numVertices = m_vertices.size() / stride;
		size_t curVertexOffset = 0;
		size_t curShape = 0;
		for(auto& s : m_shapes)
		{
			if (s.vertexOffset != curVertexOffset)
				throw std::runtime_error("shape vertex offset is not tightly packed for shape " + std::to_string(curShape));
			if (s.indexOffset % 3 != 0) 
				throw std::runtime_error("shape index offset is not a multiple of 3 for shape " + std::to_string(curShape));
			if (s.indexOffset >= numIndices)
				throw std::runtime_error("shape index offset out of range for shape " + std::to_string(curShape));
			if (s.indexCount % 3 != 0)
				throw std::runtime_error("shape index count is not a multiple of 3 for shape " + std::to_string(curShape));
			if (s.indexOffset + s.indexCount > numIndices)
				throw std::runtime_error("shape index count out of range for shape " + std::to_string(curShape));
			if (s.indexCount == 0)
				throw std::runtime_error("shape zero index count for shape " + std::to_string(curShape));

			// check indices for this shape
			size_t maxVertexIndex = 0;
			for(size_t i = s.indexOffset, end = s.indexOffset + s.indexCount; i != end; ++i)
			{
				maxVertexIndex = std::max(maxVertexIndex, m_indices[i]);
			}
			if (s.vertexCount != maxVertexIndex + 1)
				throw std::runtime_error("shape invalid vertex count for shape " + std::to_string(curShape));

			curVertexOffset += maxVertexIndex + 1;
			if (curVertexOffset > numVertices)
				throw std::runtime_error("shape index out of range for " + std::to_string(curShape));

			++curShape;
		}

		// empty tests
		if (m_shapes.empty())
			throw std::runtime_error("no shapes");
		if (m_vertices.empty())
			throw std::runtime_error("no vertices");
		if (m_indices.empty())
			throw std::runtime_error("no indices");
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

		uint32_t ui32 = read<uint32_t>(f);
		if (ui32 != s_version)
			throw std::runtime_error("invalid file version");

		ui32 = read<uint32_t>(f); // attributes

		// create mesh with attributes
		BinaryMesh m;
		m.m_attributes = ui32;

		// read vertices
		ui32 = read<uint32_t>(f); // num vertices
		// read vertex data
		m.m_vertices = read<float>(f, ui32);

		// read indices
		ui32 = read<uint32_t>(f); // num indices
		// read index data
		m.m_indices = read<uint32_t>(f, ui32);

		// read shapes
		ui32 = read<uint32_t>(f); // num shapes
		// read shape data
		m.m_shapes = read<Shape>(f, ui32);

		// check file end signature
		f >> sig[0] >> sig[1] >> sig[2];
		if (memcmp(sig, "EOF", 3) != 0)
			throw std::runtime_error("invalid end of file signature");

		f.close();

		return m;
	}

	inline void BinaryMesh::saveToFile(const std::string& filename) const
	{
		std::fstream f(filename, std::ios::out | std::ios::binary);

		// write signature
		f << 'B' << 'M' << 'F';

		// write version number
		write(f, s_version);

		// write attributes
		write(f, m_attributes);

		// write vertices
		write(f, uint32_t(m_vertices.size()));
		write(f, m_vertices);

		// write indices
		write(f, uint32_t(m_indices.size()));
		write(f, m_indices);

		// write shapes
		write(f, uint32_t(m_shapes.size()));
		write(f, m_shapes);

		// write end of file signature
		f << 'E' << 'O' << 'F';

		f.close();
	}

	template <class T>
	void BinaryMesh::write(std::fstream& stream, const T& value)
	{
		stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
	}

	template <class T>
	void BinaryMesh::write(std::fstream& stream, const std::vector<T>& vector)
	{
		stream.write(reinterpret_cast<const char*>(vector.data()), vector.size() * sizeof(T));
	}

	template <class T>
	T BinaryMesh::read(std::fstream& stream)
	{
		T res;
		stream.read(reinterpret_cast<char*>(&res), sizeof(T));
		return res;
	}

	template <class T>
	std::vector<T> BinaryMesh::read(std::fstream& stream, size_t count)
	{
		std::vector<T> res;
		res.resize(count);
		stream.read(reinterpret_cast<char*>(res.data()), count * sizeof(T));
		return res;
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
			// starts immediately
			dst.m_shapes[0].indexOffset = 0;
			dst.m_shapes[0].vertexOffset = 0;

			// copy indices and determine number of vertices to copy
			dst.m_indices.resize(src.indexCount);
			std::copy(
				m_indices.begin() + src.indexOffset,
				m_indices.begin() + src.indexOffset + src.indexCount,
				dst.m_indices.begin()
			);

			// copy vertices
			dst.m_vertices.resize(src.vertexCount * attributeCount);
			std::copy(
				m_vertices.begin() + src.vertexOffset * attributeCount,
				m_vertices.begin() + (src.vertexOffset + src.vertexCount) * attributeCount,
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
			auto vertexOffset = uint32_t((curVertex - m.m_vertices.begin()) / attributeCount);
			// additional index offset for shapes
			auto indexOffset = uint32_t(curIndex - m.m_indices.begin());

			curVertex = std::copy(src.m_vertices.begin(), src.m_vertices.end(), curVertex);
			curIndex = std::copy(src.m_indices.begin(), src.m_indices.end(), curIndex);
			curShape = std::transform(src.m_shapes.begin(), src.m_shapes.end(), curShape, 
				[indexOffset, vertexOffset](Shape s)
			{
				s.indexOffset += indexOffset;
				s.vertexOffset += vertexOffset;
				return s;
			});
		}

		return m;
	}
#pragma endregion
#pragma region Generation
	inline void BinaryMesh::changeAttributes(uint32_t newAttributes,
		const std::vector<std::unique_ptr<VertexGenerator>>& generators) 
	{
		while (newAttributes != m_attributes)
		{
			// everything generated?
			if ((newAttributes & m_attributes) == newAttributes)
			{
				// remove unnecessary attributes
				const auto oldVertexStride = getAttributeElementStride(m_attributes);
				const auto vertexCount = m_vertices.size() / oldVertexStride;
				const auto newVertexStride = getAttributeElementStride(newAttributes);
				std::vector<float> newVertices(newVertexStride * vertexCount);

				for (size_t i = 0; i < vertexCount; ++i)
				{
					const RefVertex src(m_attributes, const_cast<float*>(&m_vertices[i * oldVertexStride]));
					RefVertex dst(newAttributes, &newVertices[i * newVertexStride]);
					// change attributes to match destination
					src.copyAttributesTo(dst);
				}

				m_attributes = newAttributes;
				m_vertices = std::move(newVertices);
				// indices and shapes remain
				continue;
			}

			// find first promising generator
			auto gen = generators.begin();
			for (const auto end = generators.end(); gen != end; ++gen)
			{
				// can this generator be used?
				if (((*gen)->getRequiredAttributes() & m_attributes) != (*gen)->getRequiredAttributes())
					continue;

				// will this generator produce something useful?
				auto outAttribs = (*gen)->getOutputAttribute(m_attributes);
				if ((newAttributes & outAttribs) > (newAttributes & m_attributes))
					break;
			}

			if (gen == generators.end())
				throw std::runtime_error("BinaryMesh::changeAttributes no matching generator found to convert to new attributes");

			// use the generator to generate a new mesh
			BinaryMesh res;
			const auto svgen = dynamic_cast<SingleVertexGenerator*>((*gen).get());
			if (svgen != nullptr)
			{
				useVertexGenerator(*svgen);
				continue;
			}

			const auto mvgen = dynamic_cast<MultiVertexGenerator*>((*gen).get());
			if (mvgen != nullptr)
			{
				useVertexGenerator(*mvgen);
				continue;
			}

			throw std::runtime_error("BinaryMesh::changeAttributes incompatible vertex generator type");
		}
	}

	inline void BinaryMesh::useVertexGenerator(const SingleVertexGenerator& svgen)
	{
		const auto newAttributes = svgen.getOutputAttribute(m_attributes);
		const auto oldVertexStride = getAttributeElementStride(m_attributes);
		const auto vertexCount = m_vertices.size() / oldVertexStride;
		const auto newVertexStride = getAttributeElementStride(newAttributes);
		std::vector<float> newVertices(newVertexStride * vertexCount);

		// convert all vertices
		for (size_t i = 0; i < vertexCount; ++i)
		{
			const RefVertex src(m_attributes, const_cast<float*>(&m_vertices[i * oldVertexStride]));
			RefVertex dst(newAttributes, &newVertices[i * newVertexStride]);

			src.copyAttributesTo(dst);
			// apply generator
			svgen.generate(dst);
		}

		m_attributes = newAttributes;
		m_vertices = std::move(newVertices);
		// indices and shapes remain
	}

	inline void BinaryMesh::useVertexGenerator(const MultiVertexGenerator& mvgen)
	{
		expectSingleShape("BinaryMesh::useVertexGenerator");

		const auto newAttributes = mvgen.getOutputAttribute(m_attributes);
		const auto oldVertexStride = getAttributeElementStride(m_attributes);
		const auto vertexCount = m_vertices.size() / oldVertexStride;
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

		for (size_t vertIdx = 0; vertIdx < vertexCount; ++vertIdx)
		{
			triangleIndices.resize(0);
			triangles.resize(0);
			valueVertices.resize(0);
			outIndices.resize(0);

			// find all triangles that reference this vertex
			for (size_t idxIdx = 0, idxSize = m_indices.size(); idxIdx < idxSize; idxIdx += 3)
			{
				if (m_indices[idxIdx] != vertIdx && m_indices[idxIdx + 1] != vertIdx && m_indices[idxIdx + 2] != vertIdx)
					continue;

				// this triangle matches
				triangleIndices.emplace_back();

				// get rotatation amount (vertex index must be first index)
				size_t rot = 0;
				if (m_indices[idxIdx + 1] == vertIdx) rot = 1;
				if (m_indices[idxIdx + 2] == vertIdx) rot = 2;

				// get offsets from rotation
				size_t off0 = rot;
				size_t off1 = (rot + 1) % 3;
				size_t off2 = (rot + 2) % 3;

				auto& itri = triangleIndices.back();
				itri.index[0] = &newIndices[idxIdx + off0];
				itri.index[1] = &newIndices[idxIdx + off1];
				itri.index[2] = &newIndices[idxIdx + off2];

				// add triangle representation
				triangles.emplace_back();
				auto& tri = triangles.back();
				tri.vertex[0] = RefVertex(m_attributes, const_cast<float*>(&m_vertices[m_indices[idxIdx + off0] * oldVertexStride]));
				tri.vertex[1] = RefVertex(m_attributes, const_cast<float*>(&m_vertices[m_indices[idxIdx + off1] * oldVertexStride]));
				tri.vertex[2] = RefVertex(m_attributes, const_cast<float*>(&m_vertices[m_indices[idxIdx + off2] * oldVertexStride]));
			}

			if (triangles.empty()) continue; // this vertex is not used in any triangle => discard

			// use generator to generate new vertices
			outIndices.reserve(triangles.size());
			mvgen.generate(triangles, valueVertices, outIndices);

			assert(valueVertices.size());

			// add vertices
			const auto startIdx = uint32_t(newVertices.size());
			const auto startElementIdx = uint32_t(startIdx / newVertexStride);

			newVertices.resize(newVertices.size() + valueVertices.size() * newVertexStride);
			// copy vertices
			for (size_t i = 0; i < valueVertices.size(); ++i)
			{
				std::copy(
					valueVertices[i].data(),
					valueVertices[i].data() + newVertexStride,
					newVertices.data() + startIdx + i * newVertexStride);
			}

			// adjust indices
			if (valueVertices.size() == 1)
			{
				// all indices must be the same (there is only one vertex)
				for (size_t i = 0; i < triangleIndices.size(); ++i)
				{
					*triangleIndices[i].index[0] = startElementIdx;
				}
			}
			else
			{
				// multiple vertices => different output indices
				assert(outIndices.size() == triangles.size());
				assert(triangles.size() == triangleIndices.size());
				for (size_t i = 0; i < triangleIndices.size(); ++i)
				{
					*triangleIndices[i].index[0] = startElementIdx + outIndices[i];
				}
			}
		}

		m_attributes = newAttributes;
		m_vertices = std::move(newVertices);
		m_indices = std::move(newIndices);

		// shape indices: index offsets and count have not changed. only the values of the indices changed
		m_shapes[0].vertexCount = m_vertices.size() / newVertexStride;
	}

	inline void BinaryMesh::removeDuplicateVertices()
	{
		expectSingleShape("BinaryMesh::removeDuplicateVertices");

		std::unordered_map<RefVertex, std::vector<uint32_t>> map;

		const auto stride = getAttributeElementStride(m_attributes);
		const auto numVertices = uint32_t(m_vertices.size() / stride);
		assert(numVertices == m_shapes[0].vertexCount);

		for(uint32_t i = 0; i < numVertices; ++i)
		{
			const RefVertex v(m_attributes, &m_vertices[i * stride]);
			auto it = map.find(v);
			if(it != map.end())
			{
				// add this vertex index
				it->second.push_back(i);
			}
			else
			{
				map[v] = std::vector<uint32_t>(std::initializer_list<uint32_t>{ i });
			}
		}

		if (map.size() == numVertices) return; // no duplicates

		// remove duplicates
		std::vector<bool> usedVertices;
		// boolmap that indicates which vertices were already used
		usedVertices.assign(numVertices, false);

		// newIndexLookupTable[a] = b indicates that the vertex that was on (index) position a is now on (index) position b
		std::vector<uint32_t> newIndexLookupTable;
		newIndexLookupTable.resize(numVertices);

		std::vector<float> newVertices;
		newVertices.resize(map.size() * stride);
		auto curVertex = newVertices.begin();
		uint32_t curIndex = 0;

		// fill newVertices and create index lookup table
		for(size_t i = 0; i < numVertices; ++i)
		{
			// already inserted into new vertices?
			if(usedVertices[i]) continue;

			// insert into new vertices
			curVertex = std::copy(
				m_vertices.begin() + i * stride, 
				m_vertices.begin() + (i + 1) * stride, 
				curVertex);
			
			const RefVertex v(m_attributes, &m_vertices[i * stride]);
			auto it = map.find(v);
			// set bitflags and index lookup
			for(auto idx : it->second)
			{
				usedVertices[idx] = true;
				newIndexLookupTable[idx] = curIndex;
			}

			++curIndex;
		}
		m_vertices = std::move(newVertices);

		// fix indices
		for(auto& i : m_indices)
		{
			i = newIndexLookupTable[i];
		}

		// only vertex count changed (offset is 0 anyways)
		m_shapes[0].vertexCount = m_vertices.size() / stride;
	}

	inline void BinaryMesh::removeUnusedVertices()
	{
		expectSingleShape("BinaryMesh::removeUnusedVertices");

		const auto stride = getAttributeElementStride(m_attributes);
		const auto numVertices = uint32_t(m_vertices.size() / stride);
		assert(numVertices == m_shapes[0].vertexCount);
		std::vector<bool> used(numVertices, false);

		for (auto i : m_indices)
			used[i] = true;

		// create offset map
		// find the first unused one
		auto firstUnused = uint32_t(0);
		while (firstUnused < numVertices && used[firstUnused])
			++firstUnused;

		if (firstUnused == numVertices) return; // all vertices were used

		// create offset map
		// table[a - firstUnused] = b means: the vertex that was on index a is now on index b
		std::vector<uint32_t> newVertexIndexTable(numVertices - firstUnused);

		auto curIndex = firstUnused;
		for(auto i = firstUnused; i < numVertices; ++i)
		{
			newVertexIndexTable[i - firstUnused] = curIndex;
			if (used[i]) ++curIndex;
		}

		// move vertices
		for(size_t i = firstUnused; i < numVertices; ++i)
		{
			if (used[i])
				std::copy(
					m_vertices.begin() + i * stride,
					m_vertices.begin() + (i + 1) * stride,
					m_vertices.begin() + newVertexIndexTable[i - firstUnused] * stride);
		}
		m_vertices.resize(curIndex * stride);

		// adjust indices
		for(auto& i : m_indices)
		{
			if(i >= firstUnused)
			{
				i = newVertexIndexTable[i - firstUnused];
			}
		}

		// only vertex count changed
		m_shapes[0].vertexCount = m_vertices.size() / stride;
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
#pragma endregion

	inline void BinaryMesh::expectSingleShape(const std::string& operation)
	{
		if (m_shapes.size() != 1)
			throw std::runtime_error(operation + " only works if a single shape is present in the mesh");
	}
}