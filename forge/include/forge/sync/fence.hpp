/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <forge/dev/context.hpp>
#include <vulkan/vulkan.h>

namespace frg
{
	class Fence
	{
	public:
		explicit Fence(const Context& ctx, bool signaled = false);
		~Fence();

		/* non-copyable */
		Fence(const Fence&) = delete;
		Fence& operator=(const Fence&) = delete;

		/* movable */
		Fence(Fence&& other) noexcept;
		Fence& operator=(Fence&& other) noexcept;

		VkResult wait(uint64_t timeout = UINT64_MAX) const;
		VkResult reset() const;
		[[nodiscard]] bool signaled() const;

		[[nodiscard]] VkFence handle() const;

	private:
		VkFence fence = VK_NULL_HANDLE;
		VkDevice dev = VK_NULL_HANDLE;
	};
}