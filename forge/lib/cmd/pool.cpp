/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <stdexcept>
#include <forge/cmd/pool.hpp>

namespace frg
{
    CmdPool::CmdPool(const Context& ctx,
                    const uint32_t queue_family_index,
                    const bool allow_reset,
                    const bool transient)
        : dev(ctx.device()), queue_family_index(queue_family_index)
    {
        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = queue_family_index;
        pool_info.flags = 0;
        
        if (allow_reset)
            pool_info.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            
        if (transient)
            pool_info.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            
        if (vkCreateCommandPool(dev, &pool_info, nullptr, &cmd_pool) != VK_SUCCESS)
            throw std::runtime_error("failed to create command pool");
    }
    
    CmdPool::~CmdPool()
    {
        if (cmd_pool != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(dev, cmd_pool, nullptr);
            cmd_pool = VK_NULL_HANDLE;
        }
    }
    
    CmdPool::CmdPool(CmdPool&& other) noexcept
        : cmd_pool(other.cmd_pool), dev(other.dev)
    {
        other.cmd_pool = VK_NULL_HANDLE;
        other.dev = VK_NULL_HANDLE;
    }
    
    CmdPool& CmdPool::operator=(CmdPool&& other) noexcept
    {
        if (this != &other)
        {
            if (cmd_pool != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
                vkDestroyCommandPool(dev, cmd_pool, nullptr);
                
            cmd_pool = other.cmd_pool;
            dev = other.dev;
            
            other.cmd_pool = VK_NULL_HANDLE;
            other.dev = VK_NULL_HANDLE;
        }
        return *this;
    }
    
    VkResult CmdPool::reset(bool release_resources)
    {
        std::lock_guard lock(mtx);
        VkCommandPoolResetFlags flags = 0;
        if (release_resources)
            flags |= VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT;
            
        return vkResetCommandPool(dev, cmd_pool, flags);
    }
    
    VkResult CmdPool::allocate_cmd_buffers(std::vector<VkCommandBuffer>& buffers,
                                         uint32_t count,
                                         CmdLevel level)
    {
        std::lock_guard lock(mtx);
        buffers.resize(count, VK_NULL_HANDLE);
        
        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = cmd_pool;
        alloc_info.level = (level == CmdLevel::PRIMARY) ? 
                            VK_COMMAND_BUFFER_LEVEL_PRIMARY : 
                            VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        alloc_info.commandBufferCount = count;
        
        return vkAllocateCommandBuffers(dev, &alloc_info, buffers.data());
    }
    
    void CmdPool::free_cmd_buffers(const std::vector<VkCommandBuffer>& buffers)
    {
        if (buffers.empty())
            return;
            
        std::lock_guard lock(mtx);
        vkFreeCommandBuffers(dev, cmd_pool, static_cast<uint32_t>(buffers.size()), buffers.data());
    }
    
    VkCommandPool CmdPool::handle() const
    {
        return cmd_pool;
    }

    uint32_t CmdPool::get_queue_family() const
    {
        return queue_family_index;
    }
}
