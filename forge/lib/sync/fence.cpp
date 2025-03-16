/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <stdexcept>
#include <forge/sync/fence.hpp>

namespace frg
{
	Fence::Fence(const Context& ctx, bool signaled) : dev(ctx.device())
	{
		VkFenceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		
		if (signaled)
			create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		if (vkCreateFence(dev, &create_info, nullptr, &fence) != VK_SUCCESS)
			throw std::runtime_error("failed to create fence");
	}
	
	Fence::~Fence()
	{
		if (fence != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
		{
			vkDestroyFence(dev, fence, nullptr);
			fence = VK_NULL_HANDLE;
		}
	}
	
	Fence::Fence(Fence&& other) noexcept
		: fence(other.fence), dev(other.dev)
	{
		other.fence = VK_NULL_HANDLE;
		other.dev = VK_NULL_HANDLE;
	}
	
	Fence& Fence::operator=(Fence&& other) noexcept
	{
		if (this != &other)
		{
			if (fence != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
				vkDestroyFence(dev, fence, nullptr);
			
			fence = other.fence;
			dev = other.dev;
			
			other.fence = VK_NULL_HANDLE;
			other.dev = VK_NULL_HANDLE;
		}
		return *this;
	}
	
	VkResult Fence::wait(const uint64_t timeout) const
	{
		return vkWaitForFences(dev, 1, &fence, VK_TRUE, timeout);
	}
	
	// ReSharper disable once CppMemberFunctionMayBeConst
	VkResult Fence::reset()
	{
		return vkResetFences(dev, 1, &fence);
	}
	
	bool Fence::signaled() const
	{
		return vkGetFenceStatus(dev, fence) == VK_SUCCESS;
	}
	
	VkFence Fence::handle() const
	{
		return fence;
	}
}