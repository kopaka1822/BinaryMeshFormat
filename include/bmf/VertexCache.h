#pragma once
#include <vector>
#include "Attributes.h"

namespace bmf
{
	class VertexCache
	{
	public:
		/// \brief returns a vector from the cache
		static std::vector<float> get()
		{
			auto& d = data();
			if (d.empty())
				return getNewVector();

			auto res = std::move(d.back());
			d.pop_back();
			return res;
		}

		/// \brief puts a vector back into the cache
		static void store(std::vector<float>& vec)
		{
			data().push_back(std::move(vec));
		}

	private:
		static constexpr size_t MAX_ELEMENTS = getAttributeElementStride(Attributes::SIZE);
		static constexpr size_t INIT_COUNT = 16;

		static std::vector<float> getNewVector()
		{
			return std::vector<float>(MAX_ELEMENTS);
		}

		// internal static data
		static std::vector<std::vector<float>>& data()
		{
			static std::vector<std::vector<float>> init(INIT_COUNT, getNewVector());
			return init;
		}
	};
}
