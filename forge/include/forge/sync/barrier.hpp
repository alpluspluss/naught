/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <forge/dev/context.hpp>
#include <forge/sync/sync.hpp>
#include <vulkan/vulkan.h>

namespace frg
{
	struct MemoryBarrier
	{
		AccessFlag src_access = AccessFlag::NONE;
		AccessFlag dst_access = AccessFlag::NONE;

		MemoryBarrier() = default;

		MemoryBarrier(const AccessFlag src, const AccessFlag dst) : src_access(src), dst_access(dst) {}

		[[nodiscard]] VkMemoryBarrier get_vk_barrier() const;
	};

	struct BufferBarrier
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		AccessFlag src_access = AccessFlag::NONE;
		AccessFlag dst_access = AccessFlag::NONE;
		VkDeviceSize offset = 0;
		VkDeviceSize size = VK_WHOLE_SIZE;
		uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED;
		uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED;

		BufferBarrier() = default;

		BufferBarrier(VkBuffer buffer, AccessFlag src, AccessFlag dst) : buffer(buffer), src_access(src),
		                                                                 dst_access(dst) {}

		[[nodiscard]] VkBufferMemoryBarrier get_vk_barrier() const;
	};

	struct ImageBarrier
	{
		VkImage image = VK_NULL_HANDLE;
		AccessFlag src_access = AccessFlag::NONE;
		AccessFlag dst_access = AccessFlag::NONE;
		VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout new_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		uint32_t src_queue_family = VK_QUEUE_FAMILY_IGNORED;
		uint32_t dst_queue_family = VK_QUEUE_FAMILY_IGNORED;
		VkImageSubresourceRange subresource_range = {
			VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
		};

		ImageBarrier() = default;

		ImageBarrier(VkImage image,
		             AccessFlag src,
		             AccessFlag dst,
		             VkImageLayout old_layout,
		             VkImageLayout new_layout) : image(image), src_access(src), dst_access(dst),
		                                         old_layout(old_layout), new_layout(new_layout) {}

		[[nodiscard]] VkImageMemoryBarrier get_vk_barrier() const;
	};

	/* cmdbuffer helper */
	namespace barrier
	{
		/* ins a memory barrier */
		void memory_barrier(VkCommandBuffer cmd, const MemoryBarrier &barrier,
		                    PipelineStage src_stage, PipelineStage dst_stage);

		/* ins a buffer memory barrier */
		void buffer_barrier(VkCommandBuffer cmd, const BufferBarrier &barrier,
		                    PipelineStage src_stage, PipelineStage dst_stage);

		/* ins an image memory barrier */
		void image_barrier(VkCommandBuffer cmd, const ImageBarrier &barrier,
		                   PipelineStage src_stage, PipelineStage dst_stage);

		/* Multiple barriers at once */
		void memory_barriers(VkCommandBuffer cmd,
		                     const std::vector<MemoryBarrier> &barriers,
		                     PipelineStage src_stage,
		                     PipelineStage dst_stage);

		void buffer_barriers(VkCommandBuffer cmd,
		                     const std::vector<BufferBarrier> &barriers,
		                     PipelineStage src_stage,
		                     PipelineStage dst_stage);

		void image_barriers(VkCommandBuffer cmd,
		                    const std::vector<ImageBarrier> &barriers,
		                    PipelineStage src_stage,
		                    PipelineStage dst_stage);
	}
}
