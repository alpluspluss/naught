#include <forge/buf/buffer.hpp>
#include <forge/cmd/manager.hpp>
#include <forge/dev/context.hpp>
#include <forge/sync/fence.hpp>
#include <gtest/gtest.h>

class CmdBufferTest : public testing::Test
{
protected:
	void SetUp() override
	{
		frg::ContextCreateInfo info;
		info.app_name = "CmdBufferTest";
		info.flags = frg::ContextFlags::DEFAULT;
		ctx = std::make_unique<frg::Context>(info);
		cmd_mgr = std::make_unique<frg::CmdBufferManager>(*ctx);
	}

	void TearDown() override
	{
		cmd_mgr.reset();
		ctx.reset();
	}

	std::unique_ptr<frg::Context> ctx;
	std::unique_ptr<frg::CmdBufferManager> cmd_mgr;
};

TEST_F(CmdBufferTest, CreateCommandPool)
{
	auto *pool = cmd_mgr->get_cmd_pool(ctx->gpq_family());
	ASSERT_NE(pool, nullptr);
	EXPECT_NE(pool->handle(), VK_NULL_HANDLE);
	EXPECT_EQ(pool->get_queue_family(), ctx->gpq_family());
}

TEST_F(CmdBufferTest, AllocateCommandBuffer)
{
	auto *cmd_buffer = cmd_mgr->get_cmd_buffer(ctx->gpq_family(), frg::CmdLevel::PRIMARY);
	ASSERT_NE(cmd_buffer, nullptr);
	EXPECT_NE(cmd_buffer->handle(), VK_NULL_HANDLE);
	EXPECT_EQ(cmd_buffer->get_level(), frg::CmdLevel::PRIMARY);

	cmd_mgr->recycle_cmd_buffer(cmd_buffer);
}

TEST_F(CmdBufferTest, BeginAndEndCommandBuffer)
{
	auto *cmd_buffer = cmd_mgr->get_cmd_buffer(ctx->gpq_family());
	ASSERT_NE(cmd_buffer, nullptr);

	VkResult begin_result = cmd_buffer->begin();
	EXPECT_EQ(begin_result, VK_SUCCESS);
	EXPECT_TRUE(cmd_buffer->is_recording());

	VkResult end_result = cmd_buffer->end();
	EXPECT_EQ(end_result, VK_SUCCESS);
	EXPECT_FALSE(cmd_buffer->is_recording());

	cmd_mgr->recycle_cmd_buffer(cmd_buffer);
}

TEST_F(CmdBufferTest, CommandBufferReset)
{
	auto *cmd_buffer = cmd_mgr->get_cmd_buffer(ctx->gpq_family());
	ASSERT_NE(cmd_buffer, nullptr);

	ASSERT_EQ(cmd_buffer->begin(), VK_SUCCESS);
	ASSERT_EQ(cmd_buffer->end(), VK_SUCCESS);

	VkResult reset_result = cmd_buffer->reset();
	EXPECT_EQ(reset_result, VK_SUCCESS);

	EXPECT_EQ(cmd_buffer->begin(), VK_SUCCESS);
	EXPECT_TRUE(cmd_buffer->is_recording());
	EXPECT_EQ(cmd_buffer->end(), VK_SUCCESS);

	cmd_mgr->recycle_cmd_buffer(cmd_buffer);
}

TEST_F(CmdBufferTest, RecycleCommandBuffer)
{
	auto *cmd_buffer1 = cmd_mgr->get_cmd_buffer(ctx->gpq_family());
	ASSERT_NE(cmd_buffer1, nullptr);

	cmd_mgr->recycle_cmd_buffer(cmd_buffer1);

	auto *cmd_buffer2 = cmd_mgr->get_cmd_buffer(ctx->gpq_family());
	ASSERT_NE(cmd_buffer2, nullptr);

	EXPECT_EQ(cmd_buffer1, cmd_buffer2);

	cmd_mgr->recycle_cmd_buffer(cmd_buffer2);
}

TEST_F(CmdBufferTest, SecondaryCommandBuffer)
{
	auto *secondary_cmd = cmd_mgr->get_cmd_buffer(ctx->gpq_family(), frg::CmdLevel::SECONDARY);
	ASSERT_NE(secondary_cmd, nullptr);
	EXPECT_EQ(secondary_cmd->get_level(), frg::CmdLevel::SECONDARY);

	VkCommandBufferInheritanceInfo inheritance_info {};
	inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

	EXPECT_EQ(secondary_cmd->begin(&inheritance_info), VK_SUCCESS);
	EXPECT_TRUE(secondary_cmd->is_recording());

	vkCmdPipelineBarrier(
		secondary_cmd->handle(),
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		0, nullptr
	);

	EXPECT_EQ(secondary_cmd->end(), VK_SUCCESS);
	EXPECT_FALSE(secondary_cmd->is_recording());

	auto *primary_cmd = cmd_mgr->get_cmd_buffer(ctx->gpq_family(), frg::CmdLevel::PRIMARY);
	ASSERT_NE(primary_cmd, nullptr);

	EXPECT_EQ(primary_cmd->begin(), VK_SUCCESS);

	EXPECT_EQ(primary_cmd->execute_commands({secondary_cmd->handle()}), VK_SUCCESS);

	EXPECT_EQ(primary_cmd->end(), VK_SUCCESS);

	cmd_mgr->recycle_cmd_buffer(primary_cmd);
	cmd_mgr->recycle_cmd_buffer(secondary_cmd);
}
