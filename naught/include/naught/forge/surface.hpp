/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <naught/forge/context.hpp>
#include <vulkan/vulkan.h>

namespace nght::frg
{
	class Surface
	{
	public:
		Surface(const Context& ctx, void* window_handle);
		~Surface();

		/* non-copyable */
		Surface(const Surface&) = delete;
		Surface& operator=(const Surface&) = delete;

		/* movable */
		Surface(Surface&&) noexcept;
		Surface& operator=(Surface&&) noexcept;

		[[nodiscard]] VkSurfaceKHR handle() const;

	private:
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkInstance instance = VK_NULL_HANDLE;
	};
}