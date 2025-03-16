/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <forge/dev/context.hpp>
#include <vulkan/vulkan.h>

namespace frg
{
	class Semaphore
	{
	public:
		explicit Semaphore(const Context& ctx);
		~Semaphore();

		/* non-copyable */
		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;

		/* movable */
		Semaphore(Semaphore&& other) noexcept;
		Semaphore& operator=(Semaphore&& other) noexcept;

		[[nodiscard]] VkSemaphore handle() const;

	private:
		VkSemaphore semaphore = VK_NULL_HANDLE;
		VkDevice dev = VK_NULL_HANDLE;
	};

	/* Timeline semaphore (Vulkan 1.2+) */
	class TimelineSemaphore
	{
	public:
		explicit TimelineSemaphore(const Context& ctx, uint64_t initial_value = 0);
		~TimelineSemaphore();

		/* non-copyable */
		TimelineSemaphore(const TimelineSemaphore&) = delete;
		TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;

		/* movable */
		TimelineSemaphore(TimelineSemaphore&& other) noexcept;
		TimelineSemaphore& operator=(TimelineSemaphore&& other) noexcept;

		VkResult wait(uint64_t value, uint64_t timeout = UINT64_MAX) const;
		VkResult signal(uint64_t value) const;
		[[nodiscard]] uint64_t get_value() const;

		[[nodiscard]] VkSemaphore handle() const;

	private:
		VkSemaphore semaphore = VK_NULL_HANDLE;
		VkDevice dev = VK_NULL_HANDLE;
	};
}