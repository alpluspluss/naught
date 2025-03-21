/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <forge/dev/context.hpp>
#include <vulkan/vulkan.h>

namespace frg
{
	enum class BufUsage
	{
		CPU_TO_GPU,  /* staging buffer with CPU access */
		GPU_ONLY     /* dev local memory */
	 };

	class Buffer
	{
	public:
		Buffer(Context& ctx,
			  VkDeviceSize size,
			  VkBufferUsageFlags usage,
			  BufUsage mem_usage = BufUsage::GPU_ONLY);
		~Buffer();

		/* non-copyable */
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		/* movable */
		Buffer(Buffer&&) noexcept;
		Buffer& operator=(Buffer&&) noexcept;

		void upload(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
		void* map();
		void unmap();

		[[nodiscard]] VkBuffer handle() const;
		[[nodiscard]] VkDeviceSize size() const;

	private:
		VkBuffer buf = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		VkDevice dev = VK_NULL_HANDLE;
		Context& ctx_ref; /* reference to the context that owns the allocator;
			cuz we need to alloc without having fn call overhead */

		VkDeviceSize buf_size = 0;
		BufUsage usage = BufUsage::GPU_ONLY;
		bool is_mapped = false;
		void* mapped_ptr = nullptr;
	};
}