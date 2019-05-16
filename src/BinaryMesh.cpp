#include "../include/bmf/BinaryMesh.h"
#include "../dependencies/eigen/Eigen/Dense"

namespace bmf
{
#pragma region Getter
	uint32_t BinaryMesh::getAttributes() const
	{
		return m_attributes;
	}

	std::vector<float>& BinaryMesh::getVertices()
	{
		return m_vertices;
	}

	const std::vector<float>& BinaryMesh::getVertices() const
	{
		return m_vertices;
	}

	std::vector<uint32_t>& BinaryMesh::getIndices()
	{
		return m_indices;
	}

	const std::vector<uint32_t>& BinaryMesh::getIndices() const
	{
		return m_indices;
	}

	std::vector<BinaryMesh::Shape>& BinaryMesh::getShapes()
	{
		return m_shapes;
	}

	const std::vector<BinaryMesh::Shape>& BinaryMesh::getShapes() const
	{
		return m_shapes;
	}

	BoundingBox& BinaryMesh::getBoundingBox()
	{
		return m_bbox;
	}

	const BoundingBox& BinaryMesh::getBoundingBox() const
	{
		return m_bbox;
	}

	/*std::vector<BinaryMesh::InstanceData>& BinaryMesh::getInstanceTransforms()
	{
		return m_instances;
	}

	const std::vector<BinaryMesh::InstanceData>& BinaryMesh::getInstanceTransforms() const
	{
		return m_instances;
	}*/

	uint32_t BinaryMesh::getAttributeByteOffset(Attributes a) const
	{
		return bmf::getAttributeByteOffset(m_attributes, a);
	}

	uint32_t BinaryMesh::getAttributeByteStride() const
	{
		return bmf::getAttributeByteOffset(m_attributes, Attributes::SIZE);
	}

	uint32_t BinaryMesh::getNumVertices() const
	{
		const auto stride = getAttributeElementStride(m_attributes);
		return uint32_t(m_vertices.size() / stride);
	}

	void BinaryMesh::verify() const
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
		size_t curInstanceOffset = 0;
		size_t curShape = 0;
		BoundingBox globalBbox = -BoundingBox::max();
		try
		{
			for (auto& s : m_shapes)
			{
				if (s.vertexOffset != curVertexOffset)
					throw std::runtime_error("shape vertex offset is not tightly packed");
				//if (s.instanceOffset != curInstanceOffset)
				//	throw std::runtime_error("shape instance offset is not tightly packed");
				if (s.indexOffset % 3 != 0)
					throw std::runtime_error("shape index offset is not a multiple of 3");
				if (s.indexOffset >= numIndices)
					throw std::runtime_error("shape index offset out of range");
				if (s.indexCount % 3 != 0)
					throw std::runtime_error("shape index count is not a multiple of 3");
				if (s.indexOffset + s.indexCount > numIndices)
					throw std::runtime_error("shape index count out of range");
				if (s.indexCount == 0)
					throw std::runtime_error("shape zero index count");
				//if (s.instanceCount == 0)
				//	throw std::runtime_error("shape zero instances count");
				if (s.vertexCount == 0)
					throw std::runtime_error("shape zero vertex count");

				// check indices for this shape
				uint32_t maxVertexIndex = 0;
				for (size_t i = s.indexOffset, end = s.indexOffset + s.indexCount; i != end; ++i)
				{
					maxVertexIndex = std::max(maxVertexIndex, m_indices[i]);
				}
				if (s.vertexCount != maxVertexIndex + 1)
					throw std::runtime_error("shape invalid vertex count");

				curVertexOffset += maxVertexIndex + 1;
				if (curVertexOffset > numVertices)
					throw std::runtime_error("shape index out of range");

				// check bounding boxes
				if (m_attributes & Position)
				{
					if (s.bbox != calcBoundingBox(s))
						throw std::runtime_error("shape bounding box not correct");
					globalBbox = globalBbox.unionWith(s.bbox);
				}

				// check instances for this shape
				//curInstanceOffset += s.instanceCount;
				//if (curInstanceOffset > m_instances.size())
				//	throw std::runtime_error("shape instance count out of range");

				++curShape;
			}
		}
		catch(const std::runtime_error& e)
		{
			throw std::runtime_error(e.what() + std::string(" for shape ") + std::to_string(curShape));
		}	

		// empty tests
		if (m_shapes.empty())
			throw std::runtime_error("no shapes");
		if (m_vertices.empty())
			throw std::runtime_error("no vertices");
		if (m_indices.empty())
			throw std::runtime_error("no indices");

		// check global bbox
		if (m_attributes & Position)
		{
			if (m_bbox != globalBbox)
				throw std::runtime_error("global bounding box not correct");
		}
	}
#pragma endregion
#pragma region FileIO
	BinaryMesh BinaryMesh::loadFromFile(const std::string& filename)
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
		m.m_bbox = read<BoundingBox>(f);

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

		// read instances
		//ui32 = read<uint32_t>(f);
		//m.m_instances = read<InstanceData>(f, ui32);

		// check file end signature
		f >> sig[0] >> sig[1] >> sig[2];
		if (memcmp(sig, "EOF", 3) != 0)
			throw std::runtime_error("invalid end of file signature");

		f.close();

		return m;
	}

	void BinaryMesh::saveToFile(const std::string& filename) const
	{
		std::fstream f(filename, std::ios::out | std::ios::binary);

		// write signature
		f << 'B' << 'M' << 'F';

		// write version number
		write(f, s_version);

		// write attributes
		write(f, m_attributes);
		write(f, m_bbox);

		// write vertices
		write(f, uint32_t(m_vertices.size()));
		write(f, m_vertices);

		// write indices
		write(f, uint32_t(m_indices.size()));
		write(f, m_indices);

		// write shapes
		write(f, uint32_t(m_shapes.size()));
		write(f, m_shapes);

		//write(f, uint32_t(m_instances.size()));
		//write(f, m_instances);

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
	std::vector<BinaryMesh> BinaryMesh::splitShapes() const
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
			dst.m_bbox = src.bbox;
			//dst.m_shapes[0].instanceOffset = 0;

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

			// copy instances
			/*dst.m_instances.resize(src.instanceCount);
			std::copy(
				m_instances.begin() + src.instanceOffset,
				m_instances.begin() + src.instanceOffset + src.instanceCount,
				dst.m_instances.begin()
			);*/
		}

		return meshes;
	}

	BinaryMesh BinaryMesh::mergeShapes(const std::vector<BinaryMesh>& meshes)
	{
		if (meshes.empty()) return BinaryMesh();

		BinaryMesh m;
		m.m_attributes = meshes.front().m_attributes;
		m.m_bbox = -BoundingBox::max();
		const auto attributeCount = getAttributeElementStride(meshes.front().m_attributes);
		size_t m_totalVertices = 0;
		size_t m_totalIndices = 0;
		size_t m_totalShapes = 0;
		size_t m_totalInstances = 0;

		// acquire total vertex/index count
		for (const auto& src : meshes)
		{
			if (src.m_attributes != m.m_attributes)
				throw std::runtime_error("BinaryMesh::mergeShapes attributes of all meshes must be the same");

			m_totalVertices += src.m_vertices.size();
			m_totalIndices += src.m_indices.size();
			m_totalShapes += src.m_shapes.size();
			//m_totalInstances += src.m_instances.size();
		}

		// resize buffers
		m.m_shapes.resize(m_totalShapes);
		m.m_indices.resize(m_totalIndices);
		m.m_vertices.resize(m_totalVertices);
		//m.m_instances.resize(m_totalInstances);

		auto curShape = m.m_shapes.begin();
		auto curIndex = m.m_indices.begin();
		auto curVertex = m.m_vertices.begin();
		//auto curInstance = m.m_instances.begin();

		// copy data
		for (const auto& src : meshes)
		{
			// start index of the new vertices
			auto vertexOffset = uint32_t((curVertex - m.m_vertices.begin()) / attributeCount);
			// additional index offset for shapes
			auto indexOffset = uint32_t(curIndex - m.m_indices.begin());
			//auto instanceOffset = uint32_t(curInstance - m.m_instances.begin());

			curVertex = std::copy(src.m_vertices.begin(), src.m_vertices.end(), curVertex);
			curIndex = std::copy(src.m_indices.begin(), src.m_indices.end(), curIndex);
			//curInstance = std::copy(src.m_instances.begin(), src.m_instances.end(), curInstance);
			
			curShape = std::transform(src.m_shapes.begin(), src.m_shapes.end(), curShape,
				[&](Shape s)
			{
				s.indexOffset += indexOffset;
				s.vertexOffset += vertexOffset;
				m.m_bbox = m.m_bbox.unionWith(s.bbox);
				//s.instanceOffset += instanceOffset;
				return s;
			});
		}

		return m;
	}
#pragma endregion
#pragma region Generation
	void BinaryMesh::changeAttributes(uint32_t newAttributes,
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

	void BinaryMesh::useVertexGenerator(const SingleVertexGenerator& svgen)
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

	void BinaryMesh::useVertexGenerator(const MultiVertexGenerator& mvgen)
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
		m_shapes[0].vertexCount = uint32_t(m_vertices.size() / newVertexStride);
	}

	BoundingBox BinaryMesh::calcBoundingBox(const Shape& s) const
	{
		const auto stride = getAttributeElementStride(m_attributes);
		return getBoundingBox(
			m_vertices.data() + s.vertexOffset * stride,
			m_vertices.data() + (s.vertexOffset + s.vertexCount) * stride,
			m_attributes
		);
	}

	void BinaryMesh::removeDuplicateVertices()
	{
		expectSingleShape("BinaryMesh::removeDuplicateVertices");

		std::unordered_map<RefVertex, std::vector<uint32_t>> map;

		const auto stride = getAttributeElementStride(m_attributes);
		const auto numVertices = uint32_t(m_vertices.size() / stride);
		assert(numVertices == m_shapes[0].vertexCount);

		for (uint32_t i = 0; i < numVertices; ++i)
		{
			const RefVertex v(m_attributes, &m_vertices[i * stride]);
			auto it = map.find(v);
			if (it != map.end())
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
		for (size_t i = 0; i < numVertices; ++i)
		{
			// already inserted into new vertices?
			if (usedVertices[i]) continue;

			// insert into new vertices
			curVertex = std::copy(
				m_vertices.begin() + i * stride,
				m_vertices.begin() + (i + 1) * stride,
				curVertex);

			const RefVertex v(m_attributes, &m_vertices[i * stride]);
			auto it = map.find(v);
			// set bitflags and index lookup
			for (auto idx : it->second)
			{
				usedVertices[idx] = true;
				newIndexLookupTable[idx] = curIndex;
			}

			++curIndex;
		}
		m_vertices = std::move(newVertices);

		// fix indices
		for (auto& i : m_indices)
		{
			i = newIndexLookupTable[i];
		}

		// only vertex count changed (offset is 0 anyways)
		m_shapes[0].vertexCount = uint32_t(m_vertices.size() / stride);
	}

	void BinaryMesh::removeUnusedVertices()
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
		for (auto i = firstUnused; i < numVertices; ++i)
		{
			newVertexIndexTable[i - firstUnused] = curIndex;
			if (used[i]) ++curIndex;
		}

		// move vertices
		for (size_t i = firstUnused; i < numVertices; ++i)
		{
			if (used[i])
				std::copy(
					m_vertices.begin() + i * stride,
					m_vertices.begin() + (i + 1) * stride,
					m_vertices.begin() + newVertexIndexTable[i - firstUnused] * stride);
		}
		m_vertices.resize(curIndex * stride);

		// adjust indices
		for (auto& i : m_indices)
		{
			if (i >= firstUnused)
			{
				i = newVertexIndexTable[i - firstUnused];
			}
		}

		// only vertex count changed
		m_shapes[0].vertexCount = uint32_t(m_vertices.size() / stride);
	}

	void BinaryMesh::generateBoundingBoxes()
	{
		m_bbox = -BoundingBox::max();
		for(auto& s : m_shapes)
		{
			s.bbox = calcBoundingBox(s);
			m_bbox = m_bbox.unionWith(s.bbox);
		}
	}

	/*void BinaryMesh::deinstanceShapes(std::vector<BinaryMesh>& meshes, float epsilon)
	{
		for(const auto& m : meshes)
			m.expectSingleShape("BinaryMesh::deinstanceShapes");

		auto end = meshes.end();
		for(auto cur = meshes.begin(); cur != end; ++cur)
		{
			end = std::remove_if(cur + 1, end, [&left = *cur, epsilon](const BinaryMesh& right)
			{
				// matching attributes?
				if (left.getAttributes() != right.getAttributes()) return false;
				if (left.m_shapes[0].materialId != right.m_shapes[0].materialId) return false;
				// test for same vertex/index sizes
				if (left.m_indices.size() != right.m_indices.size()) return false;
				if (left.m_vertices.size() != right.m_vertices.size()) return false;

				// is the index pattern the same?
				if (!std::equal(left.m_indices.begin(), left.m_indices.end(), right.m_indices.begin())) return false;

				// one final check: at least four different vertices are required
				if (left.getNumVertices() < 4) return false;

				// looks good. try to recreate the transformation matrix
				// extract points from left
				const auto stride = getAttributeElementStride(left.getAttributes());
				// take vertices that are not very close to each other to improve precision
				
				// left + translation = right
				const float* l0 = left.m_vertices.data();
				const float* r0 = right.m_vertices.data();
				// determine lower and upper bound for translation for each vertex
				glm::vec3 transMin;
				glm::vec3 transMax;
				transMax.x = transMin.x = r0[0] - l0[0];
				transMax.y = transMin.y = r0[1] - l0[1];
				transMax.z = transMin.z = r0[2] - l0[2];

				// go through all vertices
				for(const float* curLeft = left.m_vertices.data(), *curRight = right.m_vertices.data(), 
					*end = left.m_vertices.data() + left.m_vertices.size();
						curLeft < end; curLeft += stride, curRight += stride)
				{
					transMin[0] = std::min(transMin[0], curRight[0] - curLeft[0]);
					transMin[1] = std::min(transMin[1], curRight[1] - curLeft[1]);
					transMin[2] = std::min(transMin[2], curRight[2] - curLeft[2]);
					transMax[0] = std::max(transMax[0], curRight[0] - curLeft[0]);
					transMax[1] = std::max(transMax[1], curRight[1] - curLeft[1]);
					transMax[2] = std::max(transMax[2], curRight[2] - curLeft[2]);
				}

				// if the difference between min an max is not high the translation should work
				auto diff = transMax - transMin;
				auto error = glm::dot(diff, diff);
				// error too high?
				if (error > epsilon) return false;

				// this is an instance => transport instance information to left
				left.m_shapes[0].instanceCount += right.m_shapes[0].instanceCount;
				std::transform(right.m_instances.begin(), right.m_instances.end(), std::back_inserter(left.m_instances),
					[trans = (transMin + transMax) / 2.0f](const glm::vec3& rightTrans)
				{
					return rightTrans + trans;
				});
				return true;

				// more complex algorithm for 4x4 transforms:
				/*const auto step = left.m_vertices.size() / stride / 4;

				const float* l0 = left.m_vertices.data();
				const float* l1 = left.m_vertices.data() + stride * step;
				const float* l2 = left.m_vertices.data() + 2 * stride * step;
				const float* l3 = left.m_vertices.data() + 3 * stride * step;
				const float* r0 = right.m_vertices.data();
				const float* r1 = right.m_vertices.data() + stride * step;
				const float* r2 = right.m_vertices.data() + 2 * stride * step;
				const float* r3 = right.m_vertices.data() + 3 * stride * step;

				glm::mat4 transformMatrix;
				// column major matrix
				transformMatrix[0][3] = 0.0f;
				transformMatrix[1][3] = 0.0f;
				transformMatrix[2][3] = 0.0f;
				transformMatrix[3][3] = 1.0f;

				Eigen::Matrix4f mat;
				mat <<
					l0[0], l0[1], l0[2], 1,
					l1[0], l1[1], l1[2], 1,
					l2[0], l2[1], l2[2], 1,
					l3[0], l3[1], l3[2], 1;
				
				// create qr decomposition
				const Eigen::ColPivHouseholderQR<Eigen::Matrix4f> decomp(mat);

				// solve linear equation to determine the transformation matrix
				for(size_t row = 0; row < 3; ++row)
				{
					Eigen::Vector4f vec{ r0[row], r1[row], r2[row], r3[row] };

					auto res = decomp.solve(vec);
					transformMatrix[0][glm::length_t(row)] = res[0];
					transformMatrix[1][glm::length_t(row)] = res[1];
					transformMatrix[2][glm::length_t(row)] = res[2];
					transformMatrix[3][glm::length_t(row)] = res[3];
				}
				
				// test if this is really the transformation matrix
				for(size_t i = 0; i < left.m_vertices.size(); i += stride)
				{
					auto lpoint = toVec3(left.m_vertices.data() + i);
					auto rpoint = toVec3(right.m_vertices.data() + i);

					// transform
					auto tpoint = transformMatrix * glm::vec4(lpoint, 1.0f);

					auto errVec = tpoint - glm::vec4(rpoint, 1.0f);
					auto error = glm::dot(errVec, errVec); // squard error

					// test if error is small enough
					if (error > epsilon) return false;
				}

				// this is an instance => transport instance information to left
				left.m_shapes[0].instanceCount += right.m_shapes[0].instanceCount;
				std::transform(right.m_instances.begin(), right.m_instances.end(), std::back_inserter(left.m_instances), 
					[&transformMatrix](const glm::mat4& mat)
				{
					return mat * transformMatrix;
				});

				return true;*//*
			});
		}

		// resize meshes
		if (end == meshes.end()) return;
		// erase meshes that became instances of another
		meshes.erase(end, meshes.end());
	}

	void BinaryMesh::centerShapes()
	{
		expectSingleShape("BinaryMesh::centerShapes");

		auto bbox = getBoundingBox(m_vertices, m_attributes);
		
		glm::vec3 center = { bbox.maxX + bbox.minX, bbox.maxY + bbox.minY, bbox.maxZ + bbox.minZ };
		center /= 2.0f;

		// move every vertex
		const auto stride = getAttributeElementStride(m_attributes);
		for(float* start = m_vertices.data(), *end = m_vertices.data() + m_vertices.size(); start < end; start += stride)
		{
			start[0] -= center.x;
			start[1] -= center.y;
			start[2] -= center.z;
		}

		// move instances
		for(auto& i : m_instances)
		{
			i += center;
		}
	}*/

	BoundingBox BinaryMesh::getBoundingBox(const float* start, const float* end,
		uint32_t attributes)
	{
		if (!(attributes & Position))
			throw std::runtime_error("positions are required to calculate bounding box");

		BoundingBox r;
		r.minX = r.minY = r.minZ = std::numeric_limits<float>::max();
		r.maxX = r.maxY = r.maxZ = -std::numeric_limits<float>::max();
		const auto stride = getAttributeElementStride(attributes);

		while (start < end)
		{
			r.minX = std::min(r.minX, start[0]);
			r.minY = std::min(r.minY, start[1]);
			r.minZ = std::min(r.minZ, start[2]);
			r.maxX = std::max(r.maxX, start[0]);
			r.maxY = std::max(r.maxY, start[1]);
			r.maxZ = std::max(r.maxZ, start[2]);

			start += stride;
		}
		return r;
	}

	BoundingBox BinaryMesh::getBoundingBox(const std::vector<float>& vertices, uint32_t attributes)
	{
		return getBoundingBox(vertices.data(), vertices.data() + vertices.size(), attributes);
	}
#pragma endregion
#pragma region Ctor
	BinaryMesh::BinaryMesh(uint32_t attributes, std::vector<float> vertices, std::vector<uint32_t> indices,
		std::vector<Shape> shapes/*, std::vector<InstanceData> instances*/)
		:
		m_vertices(move(vertices)),
		m_indices(move(indices)),
		m_shapes(move(shapes)),
		m_attributes(attributes)
		//m_instances(move(instances))
	{
	}
#pragma endregion

	void BinaryMesh::expectSingleShape(const std::string& operation) const
	{
		if (m_shapes.size() != 1)
			throw std::runtime_error(operation + " only works if a single shape is present in the mesh");
	}
}