/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <vector>
#include <forge/sync/barrier.hpp>

namespace frg
{
	VkMemoryBarrier MemoryBarrier::get_vk_barrier() const
	{
		VkMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = to_vk_access_flags(src_access);
		barrier.dstAccessMask = to_vk_access_flags(dst_access);
		return barrier;
	}

	VkBufferMemoryBarrier BufferBarrier::get_vk_barrier() const
	{
		VkBufferMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.srcAccessMask = to_vk_access_flags(src_access);
		barrier.dstAccessMask = to_vk_access_flags(dst_access);
		barrier.srcQueueFamilyIndex = src_queue_family;
		barrier.dstQueueFamilyIndex = dst_queue_family;
		barrier.buffer = buffer;
		barrier.offset = offset;
		barrier.size = size;
		return barrier;
	}

	VkImageMemoryBarrier ImageBarrier::get_vk_barrier() const
	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = to_vk_access_flags(src_access);
		barrier.dstAccessMask = to_vk_access_flags(dst_access);
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = src_queue_family;
		barrier.dstQueueFamilyIndex = dst_queue_family;
		barrier.image = image;
		barrier.subresourceRange = subresource_range;
		return barrier;
	}

	namespace barrier
	{
		void memory_barrier(VkCommandBuffer cmd,
		                    const MemoryBarrier &barrier,
		                    const PipelineStage src_stage,
		                    const PipelineStage dst_stage)
		{
			const VkMemoryBarrier vk_barrier = barrier.get_vk_barrier();

			vkCmdPipelineBarrier(
				cmd,
				to_vk_pipeline_stage(src_stage),
				to_vk_pipeline_stage(dst_stage),
				0,
				1, &vk_barrier,
				0, nullptr,
				0, nullptr
			);
		}

		void buffer_barrier(VkCommandBuffer cmd,
		                    const BufferBarrier &barrier,
		                    PipelineStage src_stage,
		                    PipelineStage dst_stage)
		{
			VkBufferMemoryBarrier vk_barrier = barrier.get_vk_barrier();

			vkCmdPipelineBarrier(
				cmd,
				to_vk_pipeline_stage(src_stage),
				to_vk_pipeline_stage(dst_stage),
				0,
				0, nullptr,
				1, &vk_barrier,
				0, nullptr
			);
		}

		void image_barrier(VkCommandBuffer cmd,
		                   const ImageBarrier &barrier,
		                   PipelineStage src_stage,
		                   PipelineStage dst_stage)
		{
			VkImageMemoryBarrier vk_barrier = barrier.get_vk_barrier();

			vkCmdPipelineBarrier(
				cmd,
				to_vk_pipeline_stage(src_stage),
				to_vk_pipeline_stage(dst_stage),
				0,
				0, nullptr,
				0, nullptr,
				1, &vk_barrier
			);
		}

		void memory_barriers(VkCommandBuffer cmd,
		                     const std::vector<MemoryBarrier> &barriers,
		                     PipelineStage src_stage,
		                     PipelineStage dst_stage)
		{
			std::vector<VkMemoryBarrier> vk_barriers;
			vk_barriers.reserve(barriers.size());

			for (const auto &barrier: barriers)
				vk_barriers.push_back(barrier.get_vk_barrier());

			vkCmdPipelineBarrier(
				cmd,
				to_vk_pipeline_stage(src_stage),
				to_vk_pipeline_stage(dst_stage),
				0,
				static_cast<uint32_t>(vk_barriers.size()), vk_barriers.data(),
				0, nullptr,
				0, nullptr
			);
		}

		void buffer_barriers(VkCommandBuffer cmd,
		                     const std::vector<BufferBarrier> &barriers,
		                     PipelineStage src_stage,
		                     PipelineStage dst_stage)
		{
			std::vector<VkBufferMemoryBarrier> vk_barriers;
			vk_barriers.reserve(barriers.size());

			for (const auto &barrier: barriers)
				vk_barriers.push_back(barrier.get_vk_barrier());

			vkCmdPipelineBarrier(
				cmd,
				to_vk_pipeline_stage(src_stage),
				to_vk_pipeline_stage(dst_stage),
				0,
				0, nullptr,
				static_cast<uint32_t>(vk_barriers.size()), vk_barriers.data(),
				0, nullptr
			);
		}

		void image_barriers(VkCommandBuffer cmd,
		                    const std::vector<ImageBarrier> &barriers,
		                    const PipelineStage src_stage,
		                    const PipelineStage dst_stage)
		{
			std::vector<VkImageMemoryBarrier> vk_barriers;
			vk_barriers.reserve(barriers.size());

			for (const auto &barrier: barriers)
				vk_barriers.push_back(barrier.get_vk_barrier());

			vkCmdPipelineBarrier(
				cmd,
				to_vk_pipeline_stage(src_stage),
				to_vk_pipeline_stage(dst_stage),
				0,
				0, nullptr,
				0, nullptr,
				static_cast<uint32_t>(vk_barriers.size()), vk_barriers.data()
			);
		}
	}
}
