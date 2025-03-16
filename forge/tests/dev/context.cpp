/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <gtest/gtest.h>
#include <forge/dev/context.hpp>

TEST(ContextTest, Creation)
{
	frg::ContextCreateInfo info;
	info.app_name = "ContextTest";
	info.flags = frg::ContextFlags::VALIDATION | frg::ContextFlags::GRAPHICS;

	ASSERT_NO_THROW({
		frg::Context ctx(info);
		EXPECT_NE(ctx.instance(), VK_NULL_HANDLE);
		EXPECT_NE(ctx.physical_device(), VK_NULL_HANDLE);
		EXPECT_NE(ctx.device(), VK_NULL_HANDLE);
		EXPECT_NE(ctx.gp_queue(), VK_NULL_HANDLE);
		});
}

TEST(ContextTest, FlagOperations)
{
	frg::ContextFlags a = frg::ContextFlags::GRAPHICS;
	frg::ContextFlags b = frg::ContextFlags::VALIDATION;
	frg::ContextFlags c = a | b;

	EXPECT_TRUE(frg::has_flag(c, frg::ContextFlags::GRAPHICS));
	EXPECT_TRUE(frg::has_flag(c, frg::ContextFlags::VALIDATION));
	EXPECT_FALSE(frg::has_flag(c, frg::ContextFlags::COMPUTE));
}

TEST(ContextTest, QueueFamilies)
{
	frg::ContextCreateInfo info;
	info.app_name = "QueueTest";
	info.flags = frg::ContextFlags::GRAPHICS | frg::ContextFlags::COMPUTE | frg::ContextFlags::TRANSFER;

	frg::Context ctx(info);

	// Graphics queue family should be valid
	EXPECT_NE(ctx.gpq_family(), UINT32_MAX);

	// Compute queue family should be valid (might be same as graphics)
	EXPECT_NE(ctx.compq_family(), UINT32_MAX);

	// Transfer queue family should be valid (might be same as graphics)
	EXPECT_NE(ctx.transq_family(), UINT32_MAX);
}

TEST(ContextTest, MoveOperations)
{
	frg::ContextCreateInfo info;
	info.app_name = "MoveTest";
	info.flags = frg::ContextFlags::VALIDATION | frg::ContextFlags::GRAPHICS;

	frg::Context ctx1(info);
	const VkDevice device1 = ctx1.device();

	frg::Context ctx2(std::move(ctx1));
	EXPECT_EQ(ctx2.device(), device1);
	EXPECT_EQ(ctx1.device(), VK_NULL_HANDLE); /* invalid */

	frg::Context ctx3(info);
	ctx3 = std::move(ctx2);
	EXPECT_EQ(ctx3.device(), device1);
	EXPECT_EQ(ctx2.device(), VK_NULL_HANDLE); /* invalid */
}
