/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <naught/forge/buffer.hpp>

namespace nght::frg
{
	class IndexBuf : public Buffer
	{
	public:
		IndexBuf(Context& ctx, VkDeviceSize size, BufUsage usage = BufUsage::GPU_ONLY);

		void bind(VkCommandBuffer cmd);
		void draw(VkCommandBuffer cmd, uint32_t instance_count = 1);

		void set_count(uint32_t count);
		[[nodiscard]] uint32_t count() const;

	private:
		uint32_t idx_count = 0;
	};
}