/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <naught/forge/ub.hpp>

namespace nght::frg
{
	VkDeviceSize UniformBuf::get_alignment() const
	{
		return alignment;
	}

	VkDeviceSize UniformBuf::get_min_uniform_alignment(const Context &ctx)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(ctx.physical_device(), &props);
		return props.limits.minUniformBufferOffsetAlignment;
	}

	VkDeviceSize UniformBuf::align_uniform_buffer_size(const Context &ctx, VkDeviceSize size)
	{
		const VkDeviceSize alignment = get_min_uniform_alignment(ctx);
		return (size + alignment - 1) & ~(alignment - 1);
	}
}
