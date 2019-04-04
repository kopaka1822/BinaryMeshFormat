#pragma once
// important includes for glm
#define GLM_FORCE_LEFT_HANDED
#define GLM_GTC_matrix_transform
#include "../../../dependencies/glm/glm/glm.hpp"
#include "../../../dependencies/glm/glm/gtc/type_ptr.hpp"

// extensions
namespace bmf
{
	inline glm::vec3 toVec3(const float* values)
	{
		return glm::vec3(values[0], values[1], values[2]);
	}

	inline void toFloat3(const glm::vec3& vec, float* values)
	{
		values[0] = vec[0];
		values[1] = vec[1];
		values[2] = vec[2];
	}
}
