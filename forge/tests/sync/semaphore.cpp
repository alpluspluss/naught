/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <forge/dev/context.hpp>
#include <forge/sync/semaphore.hpp>
#include <gtest/gtest.h>

class SemaphoreTest : public ::testing::Test
{
protected:
	frg::Context *ctx = nullptr;

	void SetUp() override
	{
		frg::ContextCreateInfo info;
		info.app_name = "SemaphoreTest";
		info.flags = frg::ContextFlags::GRAPHICS;

		ctx = new frg::Context(info);
	}

	void TearDown() override
	{
		delete ctx;
	}
};

TEST_F(SemaphoreTest, BinarySemaphoreCreation)
{
	ASSERT_NO_THROW({
		frg::Semaphore semaphore(*ctx);
		EXPECT_NE(semaphore.handle(), VK_NULL_HANDLE);
		});
}

TEST_F(SemaphoreTest, SemaphoreMoveOperations)
{
	frg::Semaphore semaphore1(*ctx);
	VkSemaphore handle1 = semaphore1.handle();
	EXPECT_NE(handle1, VK_NULL_HANDLE);

	frg::Semaphore semaphore2(std::move(semaphore1)); /* move */
	EXPECT_EQ(semaphore2.handle(), handle1);
	EXPECT_EQ(semaphore1.handle(), VK_NULL_HANDLE); /* orig invalid */

	frg::Semaphore semaphore3(*ctx);
	semaphore3 = std::move(semaphore2); /* move */
	EXPECT_EQ(semaphore3.handle(), handle1);
	EXPECT_EQ(semaphore2.handle(), VK_NULL_HANDLE);/* orig invalid; or should be anyway */
}

TEST_F(SemaphoreTest, TimelineSemaphoreCreation)
{
	ASSERT_NO_THROW({
		frg::TimelineSemaphore semaphore(*ctx, 0);
		EXPECT_NE(semaphore.handle(), VK_NULL_HANDLE);
		EXPECT_EQ(semaphore.get_value(), 0);
		});
}

TEST_F(SemaphoreTest, TimelineSemaphoreInitialValue)
{
	constexpr uint64_t initial_value = 42;

	ASSERT_NO_THROW({
		frg::TimelineSemaphore semaphore(*ctx, initial_value);
		EXPECT_NE(semaphore.handle(), VK_NULL_HANDLE);
		EXPECT_EQ(semaphore.get_value(), initial_value);
		});
}

TEST_F(SemaphoreTest, TimelineSemaphoreSignalWait)
{
	constexpr uint64_t initial_value = 0;
	constexpr uint64_t signal_value = 42;

	const frg::TimelineSemaphore semaphore(*ctx, initial_value);

	/* signal the semaphore */
	EXPECT_EQ(semaphore.signal(signal_value), VK_SUCCESS);

	/* check the value was updated */
	EXPECT_EQ(semaphore.get_value(), signal_value);

	/* wait on the current value; which should return immediately */
	EXPECT_EQ(semaphore.wait(signal_value, 0), VK_SUCCESS);

	/* wait on a future value with 0 timeout should timeout */
	EXPECT_NE(semaphore.wait(signal_value + 1, 0), VK_SUCCESS);
}

TEST_F(SemaphoreTest, TimelineSemaphoreMoveOperations)
{
	constexpr uint64_t initial_value = 42;

	frg::TimelineSemaphore semaphore1(*ctx, initial_value);
	VkSemaphore handle1 = semaphore1.handle();
	EXPECT_NE(handle1, VK_NULL_HANDLE);

	frg::TimelineSemaphore semaphore2(std::move(semaphore1));
	EXPECT_EQ(semaphore2.handle(), handle1);
	EXPECT_EQ(semaphore1.handle(), VK_NULL_HANDLE);
	EXPECT_EQ(semaphore2.get_value(), initial_value);

	// Move assignment
	frg::TimelineSemaphore semaphore3(*ctx, 0);
	semaphore3 = std::move(semaphore2);
	EXPECT_EQ(semaphore3.handle(), handle1);
	EXPECT_EQ(semaphore2.handle(), VK_NULL_HANDLE);
	EXPECT_EQ(semaphore3.get_value(), initial_value);
}
