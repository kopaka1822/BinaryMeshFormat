#pragma once
#include <array>
#include "Vertex.h"

namespace bmf
{
	struct Triangle
	{
		std::array<RefVertex, 3> vertex;
	};
}
