#pragma once
#include <array>
#include "Vertex.h"

namespace bmf
{
	struct Triangle
	{
		std::array<RefVertex, 3> vertex;
	};

	struct IndexTriangle
	{
		std::array<uint32_t*, 3> index;
		void rotateLeft()
		{
			std::swap(index[0], index[1]);
			std::swap(index[1], index[2]);
		}
		void rotateRight()
		{
			std::swap(index[2], index[1]);
			std::swap(index[1], index[0]);
		}
	};
}
