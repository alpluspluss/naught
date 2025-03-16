/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <forge/cmd/buffer.hpp>
#include <forge/cmd/pool.hpp>
#include <forge/dev/context.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace frg
{
    /* this handles creation and recycling of command buffers */
    class CmdBufferManager
    {
    public:
        explicit CmdBufferManager(Context &ctx);

        ~CmdBufferManager();

        /* get a command buffer for a specific queue family */
        CmdBuffer *get_cmd_buffer(uint32_t queue_family_index,
                                  CmdLevel level = CmdLevel::PRIMARY,
                                  CmdUsage usage = CmdUsage::ONE_TIME_SUBMIT);

        /* return a command buffer to the pool for recycling */
        void recycle_cmd_buffer(CmdBuffer *cmd_buffer);

        /* get command pool for a specific queue family - thread-safe */
        CmdPool *get_cmd_pool(uint32_t queue_family_index, bool allow_reset = true);

    private:
        Context &ctx_ref;
        std::unordered_map<uint32_t, std::unique_ptr<CmdPool> > cmd_pools;
        std::mutex pools_mtx; /* protects cmd_pools map access */

        /* maps to track free/in-use command buffers per pool and usage */
        struct BufferKey
        {
            uint32_t queue_family;
            CmdUsage usage;
            CmdLevel level;

            bool operator==(const BufferKey &other) const
            {
                return queue_family == other.queue_family &&
                       usage == other.usage &&
                       level == other.level;
            }
        };

        struct BufferKeyHash
        {
            std::size_t operator()(const BufferKey &key) const
            {
                return std::hash<uint32_t>()(key.queue_family) ^
                       (std::hash<int>()(static_cast<int>(key.usage)) << 1) ^
                       (std::hash<int>()(static_cast<int>(key.level)) << 2);
            }
        };

        std::unordered_map<BufferKey, std::vector<std::unique_ptr<CmdBuffer> >, BufferKeyHash> free_cmd_buffers;
        std::mutex free_buffers_mtx; /* protects free_cmd_buffers */
    };
}
