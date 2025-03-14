/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <vector>
#include <necs/types.hpp>

namespace necs
{
	uint64_t archash(const std::vector<Component>& components);
}