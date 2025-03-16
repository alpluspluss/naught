/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <stdexcept>
#include <forge/sync/semaphore.hpp>

namespace frg
{
	Semaphore::Semaphore(const Context& ctx) : dev(ctx.device())
	{
		VkSemaphoreCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		if (vkCreateSemaphore(dev, &create_info, nullptr, &semaphore) != VK_SUCCESS)
			throw std::runtime_error("failed to create semaphore");
	}
	
	Semaphore::~Semaphore()
	{
		if (semaphore != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(dev, semaphore, nullptr);
			semaphore = VK_NULL_HANDLE;
		}
	}
	
	Semaphore::Semaphore(Semaphore&& other) noexcept
		: semaphore(other.semaphore), dev(other.dev)
	{
		other.semaphore = VK_NULL_HANDLE;
		other.dev = VK_NULL_HANDLE;
	}
	
	Semaphore& Semaphore::operator=(Semaphore&& other) noexcept
	{
		if (this != &other)
		{
			if (semaphore != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
				vkDestroySemaphore(dev, semaphore, nullptr);
			
			semaphore = other.semaphore;
			dev = other.dev;
			
			other.semaphore = VK_NULL_HANDLE;
			other.dev = VK_NULL_HANDLE;
		}
		return *this;
	}
	
	VkSemaphore Semaphore::handle() const
	{
		return semaphore;
	}
	
	TimelineSemaphore::TimelineSemaphore(const Context& ctx, uint64_t initial_value) 
		: dev(ctx.device())
	{
		VkSemaphoreTypeCreateInfo type_info = {};
		type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		type_info.initialValue = initial_value;
		
		VkSemaphoreCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		create_info.pNext = &type_info;
		
		if (vkCreateSemaphore(dev, &create_info, nullptr, &semaphore) != VK_SUCCESS)
			throw std::runtime_error("failed to create timeline semaphore");
	}
	
	TimelineSemaphore::~TimelineSemaphore()
	{
		if (semaphore != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(dev, semaphore, nullptr);
			semaphore = VK_NULL_HANDLE;
		}
	}
	
	TimelineSemaphore::TimelineSemaphore(TimelineSemaphore&& other) noexcept
		: semaphore(other.semaphore), dev(other.dev)
	{
		other.semaphore = VK_NULL_HANDLE;
		other.dev = VK_NULL_HANDLE;
	}
	
	TimelineSemaphore& TimelineSemaphore::operator=(TimelineSemaphore&& other) noexcept
	{
		if (this != &other)
		{
			if (semaphore != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
				vkDestroySemaphore(dev, semaphore, nullptr);
			
			semaphore = other.semaphore;
			dev = other.dev;
			
			other.semaphore = VK_NULL_HANDLE;
			other.dev = VK_NULL_HANDLE;
		}
		return *this;
	}
	
	VkResult TimelineSemaphore::wait(const uint64_t value, const uint64_t timeout) const
	{
		VkSemaphoreWaitInfo wait_info = {};
		wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		wait_info.semaphoreCount = 1;
		wait_info.pSemaphores = &semaphore;
		wait_info.pValues = &value;

		return vkWaitSemaphores(dev, &wait_info, timeout);
	}
	
	VkResult TimelineSemaphore::signal(const uint64_t value) const
	{
		VkSemaphoreSignalInfo signal_info = {};
		signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
		signal_info.semaphore = semaphore;
		signal_info.value = value;
		
		return vkSignalSemaphore(dev, &signal_info);
	}
	
	uint64_t TimelineSemaphore::get_value() const
	{
		uint64_t value = 0;
		vkGetSemaphoreCounterValue(dev, semaphore, &value);
		return value;
	}
	
	VkSemaphore TimelineSemaphore::handle() const
	{
		return semaphore;
	}
}