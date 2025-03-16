/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <forge/buf/ib.hpp>

namespace frg
{
	IndexBuf::IndexBuf(Context& ctx, VkDeviceSize size, BufUsage usage)
		: Buffer(ctx,
				size,
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				usage)
	{
		/* defaults to GPU_ONLY, but CPU_TO_GPU is also supported for dynamic buffers */
	}

	void IndexBuf::bind(VkCommandBuffer cmd) const
	{
		vkCmdBindIndexBuffer(cmd, handle(), 0, VK_INDEX_TYPE_UINT32);
	}

	void IndexBuf::draw(const VkCommandBuffer cmd, const uint32_t instance_count) const
	{
		if (idx_count == 0)
			return; /* no indice to draw; ret */

		vkCmdDrawIndexed(cmd, idx_count, instance_count, 0, 0, 0);
	}

	void IndexBuf::set_count(const uint32_t count)
	{
		idx_count = count;
	}

	uint32_t IndexBuf::count() const
	{
		return idx_count;
	}
}