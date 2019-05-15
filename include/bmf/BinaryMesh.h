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
#include "generators/glm.h"

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
			// offset in the instance array
			uint32_t instanceOffset;
			// number of instances
			uint32_t instanceCount;
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
		std::vector<glm::mat4>& getInstanceTransforms();
		const std::vector<glm::mat4>& getInstanceTransforms() const;
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
		// tries to merge shapes with the same vertex/index information. epsilon: allowed squared vertex error
		static void deinstanceShapes(std::vector<BinaryMesh>& meshes, float epsilon = 0.00001f);
		// moves all shape vertices so that they are centered around the origin
		void centerShapes();
		static BoundingBox getBoundingBox(const float* start, const float* end, uint32_t attributes);
		static BoundingBox getBoundingBox(const std::vector<float>& vertices, uint32_t attributes);
#pragma endregion
#pragma region Ctor
		BinaryMesh(uint32_t attributes, std::vector<float> vertices, std::vector<uint32_t> indices,
		           std::vector<Shape> shapes, std::vector<glm::mat4> instances);
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
		void expectSingleShape(const std::string& operation) const;

		std::vector<float> m_vertices;
		std::vector<uint32_t> m_indices;
		std::vector<Shape> m_shapes;
		std::vector<glm::mat4> m_instances;
		
		uint32_t m_attributes = 0;

		static constexpr uint32_t s_version = 2;
	};

}