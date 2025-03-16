/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <cstdint>

namespace ncs
{
	using Component = uint16_t;
	using Entity = uint64_t;
	using Generation = uint16_t;

	enum class DirtyFlags : uint64_t
	{
		NONE = 0x0,
		DIRTY = 0x1,
		ADDED = 0x2,
		REMOVED = 0x4,
		UPDATED = 0x8,
	};

	inline DirtyFlags operator|(DirtyFlags a, DirtyFlags b)
	{
		return static_cast<DirtyFlags>(
			static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
		);
	}

	inline DirtyFlags& operator|=(DirtyFlags& a, DirtyFlags b)
	{
		a = a | b;
		return a;
	}

	inline DirtyFlags operator&(DirtyFlags a, DirtyFlags b)
	{
		return static_cast<DirtyFlags>(
			static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
		);
	}

	inline DirtyFlags& operator&=(DirtyFlags& a, DirtyFlags b)
	{
		a = a & b;
		return a;
	}
	inline DirtyFlags operator~(DirtyFlags updated)
	{
		return static_cast<DirtyFlags>(~static_cast<uint64_t>(updated));
	}

	inline bool has_flag(DirtyFlags flags, DirtyFlags flag)
	{
		return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) != 0;
	}

}
