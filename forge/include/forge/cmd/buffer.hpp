/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <forge/cmd/pool.hpp>
#include <forge/sync/fence.hpp>
#include <forge/sync/semaphore.hpp>
#include <forge/sync/sync.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace frg
{
	enum class CmdUsage
	{
		ONE_TIME_SUBMIT, /* optimal for buffers used once and reset */
		REUSABLE,        /* can be reused multiple times after reset */
		SIMULTANEOUS     /* can be used by multiple threads simultaneously */
	};

	/* super simple wrapper */
	class CmdBuffer
	{
	public:
		explicit CmdBuffer(CmdPool &pool,
		          CmdLevel level = CmdLevel::PRIMARY,
		          CmdUsage usage = CmdUsage::ONE_TIME_SUBMIT);

		~CmdBuffer();

		/* non-copyable */
		CmdBuffer(const CmdBuffer &) = delete;

		CmdBuffer &operator=(const CmdBuffer &) = delete;

		/* movable */
		CmdBuffer(CmdBuffer &&) noexcept;

		CmdBuffer &operator=(CmdBuffer &&) noexcept;

		/* begin command buffer recording */
		VkResult begin(VkCommandBufferInheritanceInfo *inheritance_info = nullptr);

		/* end command buffer recording */
		VkResult end();

		/* reset command buffer */
		VkResult reset(bool release_resources = false);

		/* submit command buffer */
		VkResult submit(VkQueue queue,
		                const std::vector<VkSemaphore> &wait_semaphores = {},
		                const std::vector<VkPipelineStageFlags> &wait_stages = {},
		                const std::vector<VkSemaphore> &signal_semaphores = {},
		                VkFence fence = VK_NULL_HANDLE) const;

		/* submit command buffer with our wrapper classes */
		VkResult submit(VkQueue queue,
		                const std::vector<Semaphore *> &wait_semaphores = {},
		                const std::vector<PipelineStage> &wait_stages = {},
		                const std::vector<Semaphore *> &signal_semaphores = {},
		                const Fence *fence = nullptr) const;

		/* submit batch of command buffers */
		static VkResult submit_batch(
			VkQueue queue,
			const std::vector<VkCommandBuffer> &cmd_buffers,
			const std::vector<VkSemaphore> &wait_semaphores = {},
			const std::vector<VkPipelineStageFlags> &wait_stages = {},
			const std::vector<VkSemaphore> &signal_semaphores = {},
			VkFence fence = VK_NULL_HANDLE);

		/* execute secondary command buffers from a primary one */
		VkResult execute_commands(const std::vector<VkCommandBuffer> &secondary_cmd_buffers) const;

		/* check if recording has begun */
		[[nodiscard]] bool is_recording() const;

		/* get the raw command buffer handle */
		[[nodiscard]] VkCommandBuffer handle() const;

		/* get the pool this buffer belongs to */
		[[nodiscard]] CmdPool& get_pool() const;

		/* get the usage type of this buffer */
		[[nodiscard]] CmdUsage get_usage() const;

		/* get the level of this buffer */
		[[nodiscard]] CmdLevel get_level() const;

	private:
		VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
		VkDevice dev = VK_NULL_HANDLE;
		CmdPool &pool;
		CmdUsage usage;
		CmdLevel level;
		bool recording = false;
	};
}
