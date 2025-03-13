#pragma once

#include <vector>
#include <necs/types.hpp>

namespace necs
{
	uint64_t archash(const std::vector<Component>& components);
}