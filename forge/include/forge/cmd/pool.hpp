/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <forge/dev/context.hpp>
#include <mutex>
#include <vector>
#include <vulkan/vulkan.h>

namespace frg
{
	enum class CmdLevel
	{
		PRIMARY,
		SECONDARY
	};

	/* this shit manages memory for command buffers and reset operation or whatever that is */
	class CmdPool
	{
	public:
		CmdPool(const Context& ctx,
				uint32_t queue_family_index,
				bool allow_reset = true,
				bool transient = false);
		~CmdPool();

		/* non-copyable */
		CmdPool(const CmdPool&) = delete;
		CmdPool& operator=(const CmdPool&) = delete;

		/* movable */
		CmdPool(CmdPool&&) noexcept;
		CmdPool& operator=(CmdPool&&) noexcept;

		/* reset all command buffers in the pool */
		VkResult reset(bool release_resources = false);

		/* thread safe allocate command buffers from pool */
		VkResult allocate_cmd_buffers(std::vector<VkCommandBuffer>& buffers,
									 uint32_t count,
									 CmdLevel level = CmdLevel::PRIMARY);

		/* thread safe free allocated buffers */
		void free_cmd_buffers(const std::vector<VkCommandBuffer>& buffers);

		[[nodiscard]] VkCommandPool handle() const;

		[[nodiscard]] uint32_t get_queue_family() const;

	private:
		VkCommandPool cmd_pool = VK_NULL_HANDLE;
		VkDevice dev = VK_NULL_HANDLE;
		std::mutex mtx; /* protects command pool allocation/free operations */
		uint32_t queue_family_index = UINT32_MAX;
	};
}