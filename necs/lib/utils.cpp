/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <necs/utils.hpp>

namespace necs
{
	/* hash parameters */
	constexpr auto FNV_PRIME = 1099511628211ULL;
	constexpr auto FNV_OFFSET_BASIS = 14695981039346656037ULL;

	uint64_t archash(const std::vector<Component>& components)
	{
		if (components.empty())
			return 0; /* special case for empty vectors */

		uint64_t hash = FNV_OFFSET_BASIS;
		for (const Component comp : components)
		{
			const auto* bytes = reinterpret_cast<const uint8_t*>(&comp);
			for (size_t i = 0; i < sizeof(Component); ++i)
			{
				hash ^= bytes[i];
				hash *= FNV_PRIME;
			}
		}

		return hash;
	}
}