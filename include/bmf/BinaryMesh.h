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
#include "Sphere.h"

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
		
		//using InstanceData = glm::vec3;

#pragma region Getter
		/// \brief returns bitmask of all used attributes (see Attributes enum)
		uint32_t getAttributes() const { return m_attributes; }
		std::vector<float>& getVertices() { return m_vertices; }
		const std::vector<float>& getVertices() const { return m_vertices; }
		BoundingBox& getBoundingBox() { return m_bbox; }
		const BoundingBox& getBoundingBox() const { return m_bbox; }
		//std::vector<InstanceData>& getInstanceTransforms();
		//const std::vector<InstanceData>& getInstanceTransforms() const;
		uint32_t getNumVertices() const;
		/// \brief checks if the mesh has a valid amount of indices/vertices
		/// and no index that goes beyond the buffer.
		/// \throw std::runtime error if something is wrong
		virtual void verify() const;

		/// \brief extracts only the material information from each vertex and puts it into a buffer
		std::vector<uint32_t> getMaterialAttribBuffer() const;
#pragma endregion
#pragma region FileIO
		void loadFromFile(const std::string& filename);
		void saveToFile(const std::string& filename) const;
#pragma endregion
#pragma region Generating
		void changeAttributes(uint32_t newAttributes,
		                            const std::vector<std::unique_ptr<VertexGenerator>>& generators);
		// generate bounding boxes based on vertex positions and width, height, depth attributes if available
		virtual void generateBoundingVolumes();
		/// \brief adds the offset to each material id
		virtual void offsetMaterial(uint32_t offset);
#pragma endregion
#pragma region Ctor
		BinaryMesh(uint32_t attributes, std::vector<float> vertices);
		BinaryMesh() = default;
		virtual ~BinaryMesh() = default;
		BinaryMesh(const BinaryMesh&) = default;
		BinaryMesh& operator=(const BinaryMesh&) = default;
		BinaryMesh(BinaryMesh&&) noexcept = default;
		BinaryMesh& operator=(BinaryMesh&&) noexcept = default;
#pragma endregion
	protected:
#pragma region Generating
		// generator helpers
		void useVertexGenerator(const SingleVertexGenerator& svgen);
		virtual void useMultiVertexGenerator(const MultiVertexGenerator& mvgen);

		static BoundingBox getBillboardBoundingBox(const std::vector<float>& vertices, uint32_t attributes);
		static BoundingBox getBoundingBox(const float* start, const float* end, uint32_t attributes);
		static BoundingBox getBoundingBox(const std::vector<float>& vertices, uint32_t attributes);
		static Sphere getBillboardBoundingSphere(const std::vector<float>& vertices, uint32_t attributes);
		static Sphere getBoundingSphere(const std::vector<float>& vertices, uint32_t attributes);
		static Sphere getBoundingSphere(const float* start, const float* end, uint32_t attributes);
		static bool allPointsInSphere(const std::vector<float>& vertices, uint32_t attributes, const Sphere& s);
		static bool allPointsInSphere(const float* start, const float* end, uint32_t attributes, const Sphere& s, glm::vec3* outsidePoint = nullptr);
		static glm::vec3 getLargestDistantPoint(const float* start, const float* end, size_t stride, glm::vec3 p);
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

		// virtual functions for derived class
		virtual std::string getFileSignature() const;
		virtual void writeExtendedData(std::fstream& stream) const {}
		virtual void readExtendedData(std::fstream& stream) {}
#pragma endregion 
		virtual void verifyBoundingVolumes() const;

		std::vector<float> m_vertices;
		BoundingBox m_bbox;
		Sphere m_sphere;
		uint32_t m_attributes = 0;

		static constexpr uint32_t s_version = 8;
	};

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
		//uint32_t instanceOffset;
		// number of instances
		//uint32_t instanceCount;
		// material if for this shape
		uint32_t materialId;

		// bounding box for this shape
		BoundingBox bbox;
		// bounding sphere for this shape
		Sphere sphere;
	};

	template<class IndexT>
	class ShapeBinaryMesh final : public BinaryMesh
	{
	public:

#pragma region GETTER
		std::vector<IndexT>& getIndices() { return m_indices; }
		const std::vector<IndexT>& getIndices() const { return m_indices; }
		std::vector<Shape>& getShapes() { return m_shapes; }
		const std::vector<Shape>& getShapes() const { return m_shapes; }

		// helper for dxr structures

		/// \brief adds the corresponding vertexOffset to each index.
		// therefore, vertex offset should always be 0 if this index buffer is used
		std::vector<uint32_t> getSummedIndices() const;

		/// \brief returns an array with  the size of numIndices() / 3.
		/// and holds the shape material id per triangle
		std::vector<uint32_t> getMaterialIdPerTriangle() const;
		virtual void verify() const override;
#pragma endregion 
#pragma region Grouping
		std::vector<ShapeBinaryMesh<IndexT>> splitShapes() const;
		static ShapeBinaryMesh<IndexT> mergeShapes(const std::vector<ShapeBinaryMesh<IndexT>>& meshes);
#pragma endregion
#pragma region Generating
		void removeDuplicateVertices();
		void removeUnusedVertices();
		// tries to merge shapes with the same vertex/index information. epsilon: allowed squared vertex error
		//static void deinstanceShapes(std::vector<BinaryMesh>& meshes, float epsilon = 0.00001f);
		// moves all shape vertices so that they are centered around the origin
		//void centerShapes();
		// generates bounding boxes for all shapes
		virtual void generateBoundingVolumes() override;
		// offsets all material indices by the specified offset
		virtual void offsetMaterial(uint32_t offset) override;

		// converts mesh to a mesh with 16 bit indices (moves all data to the other mesh)
		// it is recommended to call removeUnusedVertices() before calling this method.
		std::vector<ShapeBinaryMesh<uint16_t>> force16BitIndices();
#pragma endregion
#pragma region Ctor
		ShapeBinaryMesh(uint32_t attributes, std::vector<float> vertices, std::vector<IndexT> indices,
			std::vector<Shape> shapes/*, std::vector<InstanceData> instances*/);
		ShapeBinaryMesh() = default;
		~ShapeBinaryMesh() = default;
		ShapeBinaryMesh(const ShapeBinaryMesh&) = default;
		ShapeBinaryMesh& operator=(const ShapeBinaryMesh&) = default;
		ShapeBinaryMesh(ShapeBinaryMesh&&) noexcept = default;
		ShapeBinaryMesh& operator=(ShapeBinaryMesh&&) noexcept = default;
#pragma endregion 
	private:
#pragma region Generating
		// generator helpers
		void useMultiVertexGenerator(const MultiVertexGenerator& mvgen) override;
		BoundingBox calcBoundingBox(const Shape& s) const;
		Sphere calcBoundingSphere(const Shape& s) const;
#pragma endregion 
#pragma region FileIO
		// virtual functions for derived class
		virtual std::string getFileSignature() const override;
		virtual void writeExtendedData(std::fstream& stream) const override;
		virtual void readExtendedData(std::fstream& stream) override;
#pragma endregion 

		virtual void verifyBoundingVolumes() const override;
		void expectSingleShape(const std::string& operation) const;

		std::vector<IndexT> m_indices;
		std::vector<Shape> m_shapes;

		//std::vector<InstanceData> m_instances;
	};

	using BinaryMesh16 = ShapeBinaryMesh<uint16_t>;
	using BinaryMesh32 = ShapeBinaryMesh<uint32_t>;

}