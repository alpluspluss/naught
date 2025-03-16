/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <forge/buf/vb.hpp>

namespace frg
{
	VertexBuf::VertexBuf(Context& ctx, VkDeviceSize size, BufUsage usage)
		: Buffer(ctx,
				size,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				usage)
	{
		/* defaults to GPU_ONLY, but CPU_TO_GPU is also supported for dynamic buffers */
	}

	void VertexBuf::bind(const VkCommandBuffer cmd) const
	{
		constexpr VkDeviceSize offsets[] = {};
		const VkBuffer b = handle();
		vkCmdBindVertexBuffers(cmd, 0, 1, &b, offsets);
	}

	void VertexBuf::draw(const VkCommandBuffer cmd, const uint32_t vertex_count, const uint32_t instance_count)
	{
		vkCmdDraw(cmd, vertex_count, instance_count, 0, 0);
	}
}