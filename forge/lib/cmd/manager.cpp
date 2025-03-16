// forge/lib/cmd/manager.cpp
/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <forge/cmd/manager.hpp>

namespace frg
{
	CmdBufferManager::CmdBufferManager(Context &ctx) : ctx_ref(ctx)
	{
		if (ctx.gpq_family() != UINT32_MAX)
			get_cmd_pool(ctx.gpq_family());

		if (ctx.prq_family() != UINT32_MAX && ctx.prq_family() != ctx.gpq_family())
			get_cmd_pool(ctx.prq_family());

		if (ctx.transq_family() != UINT32_MAX &&
		    ctx.transq_family() != ctx.gpq_family() &&
		    ctx.transq_family() != ctx.prq_family())
			get_cmd_pool(ctx.transq_family());

		if (ctx.compq_family() != UINT32_MAX &&
		    ctx.compq_family() != ctx.gpq_family() &&
		    ctx.compq_family() != ctx.prq_family() &&
		    ctx.compq_family() != ctx.transq_family())
			get_cmd_pool(ctx.compq_family());
	}

	CmdBufferManager::~CmdBufferManager()
	{
		std::lock_guard lock_pools(pools_mtx);
		std::lock_guard<std::mutex> lock_buffers(free_buffers_mtx);

		free_cmd_buffers.clear();
		cmd_pools.clear();
	}

	CmdBuffer *CmdBufferManager::get_cmd_buffer(uint32_t queue_family_index,
	                                            CmdLevel level,
	                                            CmdUsage usage)
	{
		BufferKey key { queue_family_index, usage, level };
		std::lock_guard lock(free_buffers_mtx);
		auto &buffer_list = free_cmd_buffers[key];

		if (!buffer_list.empty())
		{
			std::unique_ptr<CmdBuffer> buffer = std::move(buffer_list.back()); /* reuse the buffer */
			buffer_list.pop_back();
			buffer->reset(true);
			return buffer.release();
		}

		/* no reusable buffer available, create a new one */
		CmdPool *pool = get_cmd_pool(queue_family_index);
		if (!pool)
			return nullptr;

		try
		{
			return new CmdBuffer(*pool, level, usage);
		}
		catch (const std::exception &)
		{
			return nullptr;
		}
	}

	void CmdBufferManager::recycle_cmd_buffer(CmdBuffer *cmd_buffer)
	{
		if (!cmd_buffer)
			return;

		if (cmd_buffer->is_recording())
			cmd_buffer->end();

		cmd_buffer->reset(true);
		const auto &pool = cmd_buffer->get_pool();
		const uint32_t queue_family = pool.get_queue_family();
		const BufferKey key = { queue_family, cmd_buffer->get_usage(), cmd_buffer->get_level() };
		std::lock_guard lock(free_buffers_mtx);
		free_cmd_buffers[key].push_back(std::unique_ptr<CmdBuffer>(cmd_buffer));
	}

	CmdPool* CmdBufferManager::get_cmd_pool(uint32_t queue_family_index, bool allow_reset)
	{
		std::lock_guard lock(pools_mtx);

		if (const auto it = cmd_pools.find(queue_family_index);
			it != cmd_pools.end())
		{
			return it->second.get();
		}

		try
		{
			auto& pool_ref = cmd_pools[queue_family_index];
			pool_ref = std::make_unique<CmdPool>(ctx_ref, queue_family_index, allow_reset);
			return pool_ref.get();
		}
		catch (const std::exception&)
		{
			cmd_pools.erase(queue_family_index);
			return nullptr;
		}
	}
}
