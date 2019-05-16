#pragma once

namespace bmf
{
	struct BoundingBox
	{
		float minX;
		float minY;
		float minZ;
		float maxX;
		float maxY;
		float maxZ;

		bool operator==(const BoundingBox& o) const
		{
			return memcmp(this, &o, sizeof(BoundingBox)) == 0;
		}
		bool operator!=(const BoundingBox& o) const
		{
			return !(*this == o);
		}

		BoundingBox unionWith(const BoundingBox& o) const
		{
			return BoundingBox{
				std::min(minX, o.minX),
				std::min(minY, o.minY),
				std::min(minZ, o.minZ),
				std::max(maxX, o.maxX),
				std::max(maxY, o.maxY),
				std::max(maxZ, o.maxZ)
			};
		}

		bool overlappingWith(const BoundingBox& o) const
		{
			if (maxX < o.minX || minX > o.maxX) return false;
			if (maxY < o.minY || minY > o.maxY) return false;
			if (maxZ < o.minZ || minZ > o.maxZ) return false;
			return true;
		}

		BoundingBox intersectionWith(const BoundingBox& o) const
		{
			return BoundingBox{
				std::max(minX, o.minX),
				std::max(minY, o.minY),
				std::max(minZ, o.minZ),
				std::min(maxX, o.maxX),
				std::min(maxY, o.maxY),
				std::min(maxZ, o.maxZ)
			};
		}

		static BoundingBox max()
		{
			return BoundingBox{
			-std::numeric_limits<float>::max(),
			-std::numeric_limits<float>::max(),
			-std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max()
			};
		}

		BoundingBox& operator-()
		{
			minX *= -1.0f;
			minY *= -1.0f;
			minZ *= -1.0f;
			maxX *= -1.0f;
			maxY *= -1.0f;
			maxZ *= -1.0f;
			return *this;
		}
	};
}