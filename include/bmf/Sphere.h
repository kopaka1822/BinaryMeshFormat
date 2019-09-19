#pragma once
#include "generators/glm.h"

namespace bmf
{
	struct Sphere
	{
		glm::vec3 center;
		float radius;

		bool operator==(const Sphere& o) const
		{
			return center == o.center && radius == o.radius;
		}
		bool operator!=(const Sphere& o) const
		{
			return !(*this == o);
		}

		Sphere unionWith(const glm::vec3& point) const
		{
			auto toCenter = center - point;
			const float dist = length(toCenter);
			if (dist < radius) return *this; // point is already inside
			toCenter /= dist; // normalize to center

			const auto opposite = center + radius * toCenter; // point furthest away from point on the sphere

			return Sphere{
				(point + opposite) * 0.5f,
				(dist + radius) * 0.50001f
			};
		}

		bool overlappingWith(const Sphere& o) const
		{
			float distSq = glm::dot(center - o.center, center - o.center);
			float radiusSum = radius + o.radius;
			return distSq < radiusSum * radiusSum;
		}

		bool isInside(const glm::vec3& point) const
		{
			return glm::dot(point - center, point - center) <= radius * radius;
		}

		static Sphere min()
		{
			return Sphere{ glm::vec3(0.0f), 0.0f };
		}

		static Sphere max()
		{
			return Sphere{ glm::vec3(0.0f), std::numeric_limits<float>::max() };
		}
	};
}