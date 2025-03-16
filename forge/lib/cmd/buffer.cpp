/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <stdexcept>
#include <forge/cmd/buffer.hpp>

namespace frg
{
    CmdBuffer::CmdBuffer(CmdPool& pool,
                         const CmdLevel level,
                         const CmdUsage usage) : pool(pool), usage(usage), level(level)
    {
        std::vector<VkCommandBuffer> buffers;
        if (pool.allocate_cmd_buffers(buffers, 1, level) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffer");

        cmd_buffer = buffers[0];
    }

    CmdBuffer::~CmdBuffer()
    {
        if (cmd_buffer != VK_NULL_HANDLE)
        {
            const std::vector buffers = { cmd_buffer };
            pool.free_cmd_buffers(buffers);
            cmd_buffer = VK_NULL_HANDLE;
        }
    }
    
    CmdBuffer::CmdBuffer(CmdBuffer&& other) noexcept : cmd_buffer(other.cmd_buffer), dev(other.dev), pool(other.pool),
                                                       usage(other.usage), level(other.level), recording(other.recording)
    {
        other.cmd_buffer = VK_NULL_HANDLE;
        other.recording = false;
    }

    CmdBuffer& CmdBuffer::operator=(CmdBuffer&& other) noexcept
    {
        if (this != &other)
        {
            if (cmd_buffer != VK_NULL_HANDLE)
            {
                std::vector buffers = { cmd_buffer };
                pool.free_cmd_buffers(buffers);
            }
            
            cmd_buffer = other.cmd_buffer;
            dev = other.dev;

            /* small note: pool ref can't be moved */
            usage = other.usage;
            recording = other.recording;
            
            other.cmd_buffer = VK_NULL_HANDLE;
            other.recording = false;
        }
        return *this;
    }
    
    VkResult CmdBuffer::begin(VkCommandBufferInheritanceInfo* inheritance_info)
    {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        switch (usage)
        {
            case CmdUsage::ONE_TIME_SUBMIT:
                begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                break;
            case CmdUsage::REUSABLE:
                begin_info.flags = 0; /* no specific flags for reusable */
                break;
            case CmdUsage::SIMULTANEOUS:
                begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                break;
        }
        
        if (inheritance_info)
            begin_info.pInheritanceInfo = inheritance_info;
            
        VkResult result = vkBeginCommandBuffer(cmd_buffer, &begin_info);
        if (result == VK_SUCCESS)
            recording = true;
            
        return result;
    }
    
    VkResult CmdBuffer::end()
    {
        if (!recording)
            return VK_ERROR_OUT_OF_DEVICE_MEMORY; /* NOT IDEAL */
            
        VkResult result = vkEndCommandBuffer(cmd_buffer);
        if (result == VK_SUCCESS)
            recording = false;
            
        return result;
    }
    
    VkResult CmdBuffer::reset(bool release_resources)
    {
        VkCommandBufferResetFlags flags = 0;
        if (release_resources)
            flags |= VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;

        const VkResult result = vkResetCommandBuffer(cmd_buffer, flags);
        if (result == VK_SUCCESS)
            recording = false;
            
        return result;
    }
    
    VkResult CmdBuffer::submit(const VkQueue queue,
                              const std::vector<VkSemaphore>& wait_semaphores,
                              const std::vector<VkPipelineStageFlags>& wait_stages,
                              const std::vector<VkSemaphore>& signal_semaphores,
                              const VkFence fence) const
    {
        if (recording)
            return VK_ERROR_OUT_OF_DEVICE_MEMORY; /* NOT IDEAL */
            
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        if (!wait_semaphores.empty() && wait_stages.size() == wait_semaphores.size())
        {
            submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
            submit_info.pWaitSemaphores = wait_semaphores.data();
            submit_info.pWaitDstStageMask = wait_stages.data();
        }

        if (!signal_semaphores.empty())
        {
            submit_info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());
            submit_info.pSignalSemaphores = signal_semaphores.data();
        }

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd_buffer;
        return vkQueueSubmit(queue, 1, &submit_info, fence);
    }
    
    VkResult CmdBuffer::submit(const VkQueue queue,
                              const std::vector<Semaphore*>& wait_semaphores,
                              const std::vector<PipelineStage>& wait_stages,
                              const std::vector<Semaphore*>& signal_semaphores,
                              const Fence* fence) const
    {
        std::vector<VkSemaphore> wait_semaphore_handles;
        std::vector<VkPipelineStageFlags> wait_stage_flags;
        std::vector<VkSemaphore> signal_semaphore_handles;
        
        for (auto* sem : wait_semaphores)
            wait_semaphore_handles.push_back(sem->handle());
            
        for (auto stage : wait_stages)
            wait_stage_flags.push_back(to_vk_pipeline_stage(stage));
            
        for (auto* sem : signal_semaphores)
            signal_semaphore_handles.push_back(sem->handle());
            
        VkFence fence_handle = fence ? fence->handle() : VK_NULL_HANDLE;
        
        return submit(queue, wait_semaphore_handles, wait_stage_flags, 
                     signal_semaphore_handles, fence_handle);
    }
    
    VkResult CmdBuffer::submit_batch(
        const VkQueue queue,
        const std::vector<VkCommandBuffer>& cmd_buffers,
        const std::vector<VkSemaphore>& wait_semaphores,
        const std::vector<VkPipelineStageFlags>& wait_stages,
        const std::vector<VkSemaphore>& signal_semaphores,
        const VkFence fence)
    {
        if (cmd_buffers.empty())
            return VK_SUCCESS; /* no-op */
            
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        
        /* wait semaphores */
        if (!wait_semaphores.empty() && wait_stages.size() == wait_semaphores.size())
        {
            submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
            submit_info.pWaitSemaphores = wait_semaphores.data();
            submit_info.pWaitDstStageMask = wait_stages.data();
        }
        
        /* signal semaphores */
        if (!signal_semaphores.empty())
        {
            submit_info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());
            submit_info.pSignalSemaphores = signal_semaphores.data();
        }
        
        /* cmdb */
        submit_info.commandBufferCount = static_cast<uint32_t>(cmd_buffers.size());
        submit_info.pCommandBuffers = cmd_buffers.data();
        
        return vkQueueSubmit(queue, 1, &submit_info, fence);
    }
    
    VkResult CmdBuffer::execute_commands(const std::vector<VkCommandBuffer>& secondary_cmd_buffers) const
    {
        if (!recording)
            return VK_ERROR_OUT_OF_DEVICE_MEMORY;
            
        if (secondary_cmd_buffers.empty())
            return VK_SUCCESS;
            
        vkCmdExecuteCommands(cmd_buffer,
                            static_cast<uint32_t>(secondary_cmd_buffers.size()),
                            secondary_cmd_buffers.data());
                            
        return VK_SUCCESS;
    }
    
    bool CmdBuffer::is_recording() const
    {
        return recording;
    }
    
    VkCommandBuffer CmdBuffer::handle() const
    {
        return cmd_buffer;
    }

    CmdPool& CmdBuffer::get_pool() const
    {
        return pool;
    }

    CmdUsage CmdBuffer::get_usage() const
    {
        return usage;
    }

    CmdLevel CmdBuffer::get_level() const
    {
        return level;
    }
}