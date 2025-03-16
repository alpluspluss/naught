/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <forge/buf/buffer.hpp>
#include <forge/dev/context.hpp>
#include <forge/sync/barrier.hpp>
#include <gtest/gtest.h>

class BarrierTest : public ::testing::Test
{
protected:
	frg::Context *ctx = nullptr;

	void SetUp() override
	{
		frg::ContextCreateInfo info;
		info.app_name = "BarrierTest";
		info.flags = frg::ContextFlags::GRAPHICS;

		ctx = new frg::Context(info);
	}

	void TearDown() override
	{
		delete ctx;
	}
};

TEST_F(BarrierTest, MemoryBarrierCreation)
{
	frg::MemoryBarrier barrier(
		frg::AccessFlag::SHADER_WRITE,
		frg::AccessFlag::SHADER_READ
	);

	const VkMemoryBarrier vk_barrier = barrier.get_vk_barrier();
	EXPECT_EQ(vk_barrier.sType, VK_STRUCTURE_TYPE_MEMORY_BARRIER);
	EXPECT_EQ(vk_barrier.srcAccessMask, VK_ACCESS_SHADER_WRITE_BIT);
	EXPECT_EQ(vk_barrier.dstAccessMask, VK_ACCESS_SHADER_READ_BIT);
}

TEST_F(BarrierTest, BufferBarrierCreation)
{
	const frg::Buffer buffer(*ctx, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, frg::BufUsage::GPU_ONLY);

	const frg::BufferBarrier barrier(buffer.handle(), frg::AccessFlag::SHADER_WRITE, frg::AccessFlag::SHADER_READ);

	VkBufferMemoryBarrier vk_barrier = barrier.get_vk_barrier();
	EXPECT_EQ(vk_barrier.sType, VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER);
	EXPECT_EQ(vk_barrier.srcAccessMask, VK_ACCESS_SHADER_WRITE_BIT);
	EXPECT_EQ(vk_barrier.dstAccessMask, VK_ACCESS_SHADER_READ_BIT);
	EXPECT_EQ(vk_barrier.buffer, buffer.handle());
	EXPECT_EQ(vk_barrier.offset, 0);
	EXPECT_EQ(vk_barrier.size, VK_WHOLE_SIZE);
	EXPECT_EQ(vk_barrier.srcQueueFamilyIndex, VK_QUEUE_FAMILY_IGNORED);
	EXPECT_EQ(vk_barrier.dstQueueFamilyIndex, VK_QUEUE_FAMILY_IGNORED);
}

TEST_F(BarrierTest, ImageBarrierCreation)
{
	/* we don't create an actual image here as it's too complex to test;
		 the barrier structure with a dummy handle should do the trick */
	auto dummy_image = reinterpret_cast<VkImage>(123); /* **note**: invalid but safe for testing */

	frg::ImageBarrier barrier(
		dummy_image,
		frg::AccessFlag::COLOR_ATTACHMENT_WRITE,
		frg::AccessFlag::SHADER_READ,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	VkImageMemoryBarrier vk_barrier = barrier.get_vk_barrier();
	EXPECT_EQ(vk_barrier.sType, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
	EXPECT_EQ(vk_barrier.srcAccessMask, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	EXPECT_EQ(vk_barrier.dstAccessMask, VK_ACCESS_SHADER_READ_BIT);
	EXPECT_EQ(vk_barrier.oldLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	EXPECT_EQ(vk_barrier.newLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	EXPECT_EQ(vk_barrier.image, dummy_image);
	EXPECT_EQ(vk_barrier.srcQueueFamilyIndex, VK_QUEUE_FAMILY_IGNORED);
	EXPECT_EQ(vk_barrier.dstQueueFamilyIndex, VK_QUEUE_FAMILY_IGNORED);

	/* default should be set */
	EXPECT_EQ(vk_barrier.subresourceRange.aspectMask, VK_IMAGE_ASPECT_COLOR_BIT);
	EXPECT_EQ(vk_barrier.subresourceRange.baseMipLevel, 0);
	EXPECT_EQ(vk_barrier.subresourceRange.levelCount, 1);
	EXPECT_EQ(vk_barrier.subresourceRange.baseArrayLayer, 0);
	EXPECT_EQ(vk_barrier.subresourceRange.layerCount, 1);
}

TEST_F(BarrierTest, AccessFlagOperations)
{
	frg::AccessFlag a = frg::AccessFlag::SHADER_READ;
	frg::AccessFlag b = frg::AccessFlag::SHADER_WRITE;
	frg::AccessFlag c = a | b;

	EXPECT_EQ(frg::to_vk_access_flags(c),
	          VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

	frg::AccessFlag d = frg::AccessFlag::NONE;
	d |= a;
	EXPECT_EQ(frg::to_vk_access_flags(d), VK_ACCESS_SHADER_READ_BIT);
}

TEST_F(BarrierTest, PipelineStageConversion)
{
	EXPECT_EQ(frg::to_vk_pipeline_stage(frg::PipelineStage::TOP),
	          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

	EXPECT_EQ(frg::to_vk_pipeline_stage(frg::PipelineStage::FRAGMENT_SHADER),
	          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	EXPECT_EQ(frg::to_vk_pipeline_stage(frg::PipelineStage::BOTTOM),
	          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
}

/* TODO: Barrier insertion test; until command buffer is implemented */
