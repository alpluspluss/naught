/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <cstring>
#include <stdexcept>
#include <forge/buf/buffer.hpp>

namespace frg
{
	Buffer::Buffer(Context &ctx,
	               const VkDeviceSize size,
	               const VkBufferUsageFlags usage,
	               const BufUsage mem_usage) : dev(ctx.device()), ctx_ref(ctx), buf_size(size), usage(mem_usage)
	{
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo alloc_info = {};

		/* cfg memory usage based on the buffer's intended usage pattern */
		if (mem_usage == BufUsage::CPU_TO_GPU)
		{
			/* use host-visible memory for staging buffer */
			alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}
		else
		{
			/* for GPU-only buffers, use device-local memory */
			alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}

		const VkResult result = vmaCreateBuffer(ctx_ref.allocator(), &buffer_info, &alloc_info, &buf,
			&allocation, nullptr);

		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create buffer with VMA");
	}

	Buffer::~Buffer()
	{
		if (is_mapped)
			unmap();

		if (buf != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE)
		{
			vmaDestroyBuffer(ctx_ref.allocator(), buf, allocation);
			buf = VK_NULL_HANDLE;
			allocation = VK_NULL_HANDLE;
		}
	}

	Buffer::Buffer(Buffer &&other) noexcept : ctx_ref(other.ctx_ref),
	                                          buf(other.buf),
	                                          allocation(other.allocation),
	                                          dev(other.dev),
	                                          buf_size(other.buf_size),
	                                          usage(other.usage),
	                                          is_mapped(other.is_mapped),
	                                          mapped_ptr(other.mapped_ptr)
	{
		other.buf = VK_NULL_HANDLE;
		other.allocation = VK_NULL_HANDLE;
		other.is_mapped = false;
		other.mapped_ptr = nullptr;
	}

	Buffer &Buffer::operator=(Buffer &&other) noexcept
	{
		if (this != &other && &ctx_ref == &other.ctx_ref)
		{
			if (is_mapped)
				unmap();

			if (buf != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE)
				vmaDestroyBuffer(ctx_ref.allocator(), buf, allocation);

			/* note: ctx_ref cannot be reassigned since it's a reference */
			buf = other.buf;
			allocation = other.allocation;
			dev = other.dev;
			buf_size = other.buf_size;
			usage = other.usage;
			is_mapped = other.is_mapped;
			mapped_ptr = other.mapped_ptr;

			other.buf = VK_NULL_HANDLE;
			other.allocation = VK_NULL_HANDLE;
			other.is_mapped = false;
			other.mapped_ptr = nullptr;
		}
		return *this;
	}

	void Buffer::upload(const void *data, VkDeviceSize size, VkDeviceSize offset)
	{
		if (size + offset > buf_size)
			throw std::runtime_error("buffer upload exceeds buffer size");

		if (usage == BufUsage::CPU_TO_GPU)
		{
			/* direct upload for host-visible memory */
			void *mapped_data = map();
			std::memcpy(static_cast<char *>(mapped_data) + offset, data, size);
			unmap();
		}
		else
		{
			/* for device-local memory, we should use a staging buffer and commands
				requires a command buffer and transfer queue submission */
			throw std::runtime_error("direct upload to device-local memory not implemented - use a staging buffer");
		}
	}

	void *Buffer::map()
	{
		if (is_mapped)
			return mapped_ptr;

		if (usage != BufUsage::CPU_TO_GPU)
			throw std::runtime_error("cannot map a GPU-only buffer");

		void *mapped_data = nullptr;
		if (const VkResult result = vmaMapMemory(ctx_ref.allocator(), allocation, &mapped_data);
			result != VK_SUCCESS)
			throw std::runtime_error("failed to map buffer memory");

		is_mapped = true;
		mapped_ptr = mapped_data;
		return mapped_ptr;
	}

	void Buffer::unmap()
	{
		if (!is_mapped)
			return;

		vmaUnmapMemory(ctx_ref.allocator(), allocation);
		is_mapped = false;
		mapped_ptr = nullptr;
	}

	VkBuffer Buffer::handle() const
	{
		return buf;
	}

	VkDeviceSize Buffer::size() const
	{
		return buf_size;
	}
}