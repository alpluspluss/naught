/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <naught/forge/buffer.hpp>

namespace nght::frg
{
	class VertexBuf : public Buffer
	{
	public:
		VertexBuf(Context& ctx, VkDeviceSize size, BufUsage usage = BufUsage::GPU_ONLY);

		void bind(VkCommandBuffer cmd) const;
		static void draw(VkCommandBuffer cmd, uint32_t vertex_count, uint32_t instance_count = 1);

	private:
		uint32_t vert_count = 0;
	};
}