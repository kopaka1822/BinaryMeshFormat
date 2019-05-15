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
	};
}