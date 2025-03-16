/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <gtest/gtest.h>
#include <forge/dev/context.hpp>
#include <forge/sync/fence.hpp>

class FenceTest : public ::testing::Test
{
protected:
	frg::Context *ctx = nullptr;

	void SetUp() override
	{
		frg::ContextCreateInfo info;
		info.app_name = "FenceTest";
		info.flags = frg::ContextFlags::GRAPHICS;

		ctx = new frg::Context(info);
	}

	void TearDown() override
	{
		delete ctx;
	}
};

TEST_F(FenceTest, FenceCreation)
{
	ASSERT_NO_THROW({
		frg::Fence fence(*ctx, false);
		EXPECT_NE(fence.handle(), VK_NULL_HANDLE);
		});
}

TEST_F(FenceTest, SignaledFence)
{
	ASSERT_NO_THROW({
		frg::Fence fence(*ctx, true);
		EXPECT_NE(fence.handle(), VK_NULL_HANDLE);

		EXPECT_TRUE(fence.signaled());
		EXPECT_EQ(fence.wait(0), VK_SUCCESS);
		EXPECT_EQ(fence.reset(), VK_SUCCESS);
		EXPECT_FALSE(fence.signaled());
	});
}

TEST_F(FenceTest, FenceMoveOperations)
{
	frg::Fence fence1(*ctx, false);
	VkFence handle1 = fence1.handle();
	EXPECT_NE(handle1, VK_NULL_HANDLE);

	frg::Fence fence2(std::move(fence1));
	EXPECT_EQ(fence2.handle(), handle1);
	EXPECT_EQ(fence1.handle(), VK_NULL_HANDLE);

	frg::Fence fence3(*ctx, false);
	fence3 = std::move(fence2);
	EXPECT_EQ(fence3.handle(), handle1);
	EXPECT_EQ(fence2.handle(), VK_NULL_HANDLE);
}

/* TODO: Once*/
