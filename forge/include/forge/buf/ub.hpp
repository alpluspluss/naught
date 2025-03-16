/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <cstring>
#include <stdexcept>
#include <forge/buf/buffer.hpp>

namespace frg
{
	class UniformBuf : public Buffer
	{
	public:
		UniformBuf(Context& ctx, const VkDeviceSize size)
		   : Buffer(ctx,
				  align_uniform_buffer_size(ctx, size),
				  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				  BufUsage::CPU_TO_GPU), /* uniform buffers must be CPU accessible */
				  alignment(get_min_uniform_alignment(ctx))
		{
		}

		template<typename T>
		void update(const T& data)
		{
			if (sizeof(T) > size())
				throw std::runtime_error("Uniform data is larger than buffer size");

			void* mapped = map();
			std::memcpy(mapped, &data, sizeof(T));
			unmap();
		}

		[[nodiscard]] VkDeviceSize get_alignment() const;

	private:
		VkDeviceSize alignment;

		static VkDeviceSize get_min_uniform_alignment(const Context& ctx);

		static VkDeviceSize align_uniform_buffer_size(const Context& ctx, VkDeviceSize size);
	};
}