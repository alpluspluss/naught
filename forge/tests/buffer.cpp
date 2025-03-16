/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <gtest/gtest.h>
#include <forge/dev/context.hpp>
#include <forge/buf/buffer.hpp>
#include <forge/buf/vb.hpp>
#include <forge/buf/ib.hpp>
#include <forge/buf/ub.hpp>

class BufferTest : public ::testing::Test
{
protected:
	frg::Context *ctx = nullptr;

	void SetUp() override
	{
		frg::ContextCreateInfo info;
		info.app_name = "BufferTest";
		info.flags = frg::ContextFlags::GRAPHICS;

		ctx = new frg::Context(info);
	}

	void TearDown() override
	{
		delete ctx;
	}
};

TEST_F(BufferTest, GenericBufferCreation)
{
	constexpr VkDeviceSize buffer_size = 1024;

	ASSERT_NO_THROW({
		frg::Buffer buffer(*ctx, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, frg::BufUsage::CPU_TO_GPU);
		EXPECT_NE(buffer.handle(), VK_NULL_HANDLE);
		EXPECT_EQ(buffer.size(), buffer_size);
		});
}

TEST_F(BufferTest, BufferMoveOperations)
{
	constexpr VkDeviceSize buffer_size = 1024;

	frg::Buffer buffer1(*ctx, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, frg::BufUsage::CPU_TO_GPU);
	VkBuffer handle1 = buffer1.handle();
	EXPECT_NE(handle1, VK_NULL_HANDLE);

	// Move constructor
	frg::Buffer buffer2(std::move(buffer1));
	EXPECT_EQ(buffer2.handle(), handle1);
	EXPECT_EQ(buffer1.handle(), VK_NULL_HANDLE); // Original should be invalidated

	// Move assignment
	frg::Buffer buffer3(*ctx, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, frg::BufUsage::CPU_TO_GPU);
	buffer3 = std::move(buffer2);
	EXPECT_EQ(buffer3.handle(), handle1);
	EXPECT_EQ(buffer2.handle(), VK_NULL_HANDLE); // Original should be invalidated
}

TEST_F(BufferTest, BufferUploadAndMap)
{
	constexpr VkDeviceSize buffer_size = 16;
	frg::Buffer buffer(*ctx, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, frg::BufUsage::CPU_TO_GPU);

	// Data to upload
	const float test_data[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
	ASSERT_NO_THROW(buffer.upload(test_data, sizeof(test_data)));

	// Map and verify
	void *mapped_data = nullptr;
	ASSERT_NO_THROW(mapped_data = buffer.map());
	ASSERT_NE(mapped_data, nullptr);

	float *float_data = static_cast<float *>(mapped_data);
	EXPECT_FLOAT_EQ(float_data[0], 1.0f);
	EXPECT_FLOAT_EQ(float_data[1], 2.0f);
	EXPECT_FLOAT_EQ(float_data[2], 3.0f);
	EXPECT_FLOAT_EQ(float_data[3], 4.0f);

	ASSERT_NO_THROW(buffer.unmap());
}

TEST_F(BufferTest, VertexBufferCreation)
{
	constexpr VkDeviceSize buffer_size = 1024;

	ASSERT_NO_THROW({
		frg::VertexBuf vbuffer(*ctx, buffer_size, frg::BufUsage::CPU_TO_GPU);
		EXPECT_NE(vbuffer.handle(), VK_NULL_HANDLE);
		EXPECT_EQ(vbuffer.size(), buffer_size);
		});
}

TEST_F(BufferTest, IndexBufferCreation)
{
	constexpr VkDeviceSize buffer_size = 1024;

	ASSERT_NO_THROW({
		frg::IndexBuf ibuffer(*ctx, buffer_size, frg::BufUsage::CPU_TO_GPU);
		EXPECT_NE(ibuffer.handle(), VK_NULL_HANDLE);
		EXPECT_EQ(ibuffer.size(), buffer_size);

		// Set and get index count
		ibuffer.set_count(100);
		EXPECT_EQ(ibuffer.count(), 100);
		});
}

TEST_F(BufferTest, UniformBufferCreation)
{
	struct TestUniform
	{
		float data[4];
	};

	{
		constexpr VkDeviceSize buffer_size = sizeof(TestUniform);
		frg::UniformBuf ubuffer(*ctx, buffer_size);
		EXPECT_NO_THROW(EXPECT_NE(ubuffer.handle(), VK_NULL_HANDLE));
		EXPECT_GE(ubuffer.size(), buffer_size);

		constexpr TestUniform uniform = { { 1.0f, 2.0f, 3.0f, 4.0f } };
		ubuffer.update(uniform);
	}
}