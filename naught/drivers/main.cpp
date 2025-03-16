/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <iostream>
#include <forge/buf/buffer.hpp>
#include <forge/buf/ib.hpp>
#include <forge/buf/ub.hpp>
#include <forge/buf/vb.hpp>
#include <forge/dev/context.hpp>
#include <forge/dev/surface.hpp>
#include <forge/swapchain/swapchain.hpp>
#include <forge/cmd/manager.hpp>
#include <forge/sync/fence.hpp>
#include <naught/host/app.hpp>
#include <naught/host/input.hpp>
#include <naught/host/window.hpp>

struct Vertex
{
	float position[3];
	float color[3];
};

struct MVP /* ubuf test */
{
	alignas(16) float model[16];
	alignas(16) float view[16];
	alignas(16) float projection[16];
};

int main()
{
	auto &app = nght::App::get();

	auto *window = app.window({
		.title = "Naught",
		.bounds = { 900.0f, 700.0f },
		.style = nght::NaughtWindow::Style::STANDARD
	});

	try
	{
		frg::ContextCreateInfo info;
		info.app_name = "Naught";
		info.flags = frg::ContextFlags::DEFAULT;

		frg::Context context(info);

		std::cout << "vulkan context created successfully!" << std::endl;
		std::cout << "using device: " << context.device() << std::endl;

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(context.physical_device(), &props);
		std::cout << "device name: " << props.deviceName << std::endl;
		std::cout << "device type: " <<
		(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
			 ? "discrete GPU"
			 : props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
				   ? "integrated GPU"
				   : props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU
					     ? "cpu"
					     : "other") << std::endl;
		std::cout << "API version: "
				<< VK_VERSION_MAJOR(props.apiVersion) << "."
				<< VK_VERSION_MINOR(props.apiVersion) << "."
				<< VK_VERSION_PATCH(props.apiVersion) << std::endl;

		std::cout << "graphics queue family: " << context.gpq_family() << std::endl;
		std::cout << "transfer queue family: " << context.transq_family() << std::endl;
		std::cout << "compute queue family: " << context.compq_family() << std::endl;
		std::cout << "max MSAA samples: " << context.sample_count() << std::endl;

		void *native_window_handle = window->view()->native_layer();
		const frg::Surface surface(context, native_window_handle);
		std::cout << "surface created successfully! handle: " << surface.handle() << std::endl;

		const nght::Vec2 size = {
			(window->size().first),
			(window->size().second)
		};
		frg::Swapchain swapchain(context, surface, size);

		std::cout << "\nswapchain created successfully!" << std::endl;
		std::cout << "swapchain handle: " << swapchain.handle() << std::endl;
		std::cout << "swapchain format: " << swapchain.format() << std::endl;
		std::cout << "swapchain extent: " << swapchain.extent().width << "x"
				<< swapchain.extent().height << std::endl;
		std::cout << "swapchain images: " << swapchain.images().size() << std::endl;
		std::cout << "swapchain image views: " << swapchain.views().size() << std::endl;

		constexpr VkDeviceSize buf_size = 128;
		frg::Buffer cpu_buffer(
			context,
			buf_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			frg::BufUsage::CPU_TO_GPU
		);
		std::cout << "CPU buffer created: " << cpu_buffer.handle() << std::endl;

		constexpr float test_data[16] = {
			0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
			8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
		};
		cpu_buffer.upload(test_data, sizeof(test_data));

		const auto mapped_data = static_cast<float *>(cpu_buffer.map());
		std::cout << "first values: " << mapped_data[0] << ", "
				<< mapped_data[1] << ", " << mapped_data[2] << std::endl;
		cpu_buffer.unmap();

		/* device local */
		frg::Buffer gpu_buffer(
			context,
			buf_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			frg::BufUsage::GPU_ONLY
		);

		std::cout << "GPU buffer created: " << gpu_buffer.handle() << std::endl;

		constexpr Vertex triangle_verts[] = {
			{ { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};

		constexpr VkDeviceSize vertex_buffer_size = sizeof(triangle_verts);
		frg::VertexBuf vertex_buffer(
			context,
			vertex_buffer_size,
			frg::BufUsage::CPU_TO_GPU /* using CPU_TO_GPU for this demo to directly upload */
		);

		vertex_buffer.upload(triangle_verts, vertex_buffer_size);
		std::cout << "VertexBuf created: " << vertex_buffer.handle() << std::endl;

		/* quad triangle */
		constexpr uint32_t quad_indices[] = {
			0, 1, 2,
			0, 2, 3
		};

		constexpr VkDeviceSize index_buffer_size = sizeof(quad_indices);
		frg::IndexBuf index_buffer(
			context,
			index_buffer_size,
			frg::BufUsage::CPU_TO_GPU
		);

		index_buffer.upload(quad_indices, index_buffer_size);
		index_buffer.set_count(6); /* 6 / 3 = 2 triangles */
		std::cout << "IndexBuf created: " << index_buffer.handle() << std::endl;
		std::cout << "IndexBuf count: " << index_buffer.count() << std::endl;

		constexpr VkDeviceSize ubo_size = sizeof(MVP);
		frg::UniformBuf uniform_buffer(context, ubo_size);

		std::cout << "ubuf created: " << uniform_buffer.handle() << std::endl;
		std::cout << "req size: " << ubo_size << " bytes" << std::endl;
		std::cout << "actual size (aligned): " << uniform_buffer.size() << " bytes" << std::endl;
		std::cout << "alignment requirement: " << uniform_buffer.get_alignment() << " bytes" << std::endl;

		MVP mvp = {};
		for (auto i = 0; i < 3; i++)
		{
			for (auto j = 0; j < 16; j++)
			{
				float *matrix = (i == 0) ? mvp.model : (i == 1) ? mvp.view : mvp.projection;
				matrix[j] = (j % 5 == 0) ? 1.0f : 0.0f; /* diagonal */
			}
		}
		uniform_buffer.update(mvp);

		/* Command Buffer Manager Testing */
		std::cout << "\n--- Command Buffer System Testing ---\n";
		frg::CmdBufferManager cmd_mgr(context);

		/* Test 1: Buffer Copy Operation */
		std::cout << "Testing buffer copy operation...\n";
		frg::Fence fence(context, false);

		constexpr VkDeviceSize copy_buffer_size = 1024;
		frg::Buffer src_copy_buffer(context,
		                            copy_buffer_size,
		                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                            frg::BufUsage::CPU_TO_GPU);

		frg::Buffer dst_copy_buffer(context,
		                            copy_buffer_size,
		                            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		                            frg::BufUsage::GPU_ONLY);

		std::vector copy_test_data(copy_buffer_size / sizeof(uint32_t), 0xDEADBEEF);
		src_copy_buffer.upload(copy_test_data.data(), copy_buffer_size);

		if (auto *cmd_buffer = cmd_mgr.get_cmd_buffer(context.gpq_family()))
		{
			if (cmd_buffer->begin() == VK_SUCCESS)
			{
				VkBufferCopy copy_region {};
				copy_region.srcOffset = 0;
				copy_region.dstOffset = 0;
				copy_region.size = copy_buffer_size;

				vkCmdCopyBuffer(cmd_buffer->handle(),
				                src_copy_buffer.handle(),
				                dst_copy_buffer.handle(),
				                1, &copy_region);

				cmd_buffer->end();
				VkResult submit_result = cmd_buffer->submit(context.gp_queue(), {}, {}, {}, fence.handle());
				if (submit_result == VK_SUCCESS)
				{
					VkResult wait_result = fence.wait();
					if (wait_result == VK_SUCCESS)
					{
						std::cout << "Buffer copy successful!\n";
					}
					else
					{
						std::cerr << "Error waiting for fence: " << wait_result << std::endl;
					}
				}
				else
				{
					std::cerr << "Error submitting command buffer: " << submit_result << std::endl;
				}
			}
			cmd_mgr.recycle_cmd_buffer(cmd_buffer);
		}

		/* Test 2: Batch Submission */
		std::cout << "Testing batch submission...\n";
		fence.reset();

		std::vector<frg::CmdBuffer *> batch_buffers;
		std::vector<VkCommandBuffer> vk_batch_buffers;

		for (auto i = 0; i < 3; i++)
		{
			if (auto *buffer = cmd_mgr.get_cmd_buffer(context.gpq_family()))
			{
				if (buffer->begin() == VK_SUCCESS)
				{
					vkCmdPipelineBarrier(
						buffer->handle(),
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						0,
						0, nullptr,
						0, nullptr,
						0, nullptr
					);

					buffer->end();
					batch_buffers.push_back(buffer);
					vk_batch_buffers.push_back(buffer->handle());
				}
			}
		}

		if (!vk_batch_buffers.empty())
		{
			VkResult batch_result = frg::CmdBuffer::submit_batch(
				context.gp_queue(),
				vk_batch_buffers,
				{}, {}, {},
				fence.handle()
			);

			if (batch_result == VK_SUCCESS)
			{
				VkResult wait_result = fence.wait();
				if (wait_result == VK_SUCCESS)
				{
					std::cout << "Batch submission successful!\n";
				}
				else
				{
					std::cerr << "Error waiting for fence: " << wait_result << std::endl;
				}
			}
			else
			{
				std::cerr << "Error in batch submission: " << batch_result << std::endl;
			}
		}

		for (auto *buffer: batch_buffers)
		{
			cmd_mgr.recycle_cmd_buffer(buffer);
		}

		/* Test 3: Different Usage Types */
		std::cout << "Testing different command buffer usage types...\n";

		auto *one_time_cmd = cmd_mgr.get_cmd_buffer(context.gpq_family(),
		                                            frg::CmdLevel::PRIMARY,
		                                            frg::CmdUsage::ONE_TIME_SUBMIT);
		if (one_time_cmd)
		{
			std::cout << "Created ONE_TIME_SUBMIT command buffer\n";
			cmd_mgr.recycle_cmd_buffer(one_time_cmd);
		}

		auto *reusable_cmd = cmd_mgr.get_cmd_buffer(context.gpq_family(),
		                                            frg::CmdLevel::PRIMARY,
		                                            frg::CmdUsage::REUSABLE);
		if (reusable_cmd)
		{
			std::cout << "Created REUSABLE command buffer\n";
			cmd_mgr.recycle_cmd_buffer(reusable_cmd);
		}

		auto *simultaneous_cmd = cmd_mgr.get_cmd_buffer(context.gpq_family(),
		                                                frg::CmdLevel::PRIMARY,
		                                                frg::CmdUsage::SIMULTANEOUS);
		if (simultaneous_cmd)
		{
			std::cout << "Created SIMULTANEOUS command buffer\n";
			cmd_mgr.recycle_cmd_buffer(simultaneous_cmd);
		}

		std::cout << "Command buffer system tests completed\n";

		window->on_resize = [&swapchain, window]()
		{
			std::cout << "window resized to " << window->size().first << "x" << window->size().second << "\n";
			const nght::Vec2 new_size = {
				(window->size().first),
				(window->size().second)
			};
			swapchain.resize(new_size);
			std::cout << "swapchain resized to " << swapchain.extent().width << "x"
					<< swapchain.extent().height << std::endl;
		};
	}
	catch (const std::exception &e)
	{
		std::cerr << "error: " << e.what() << std::endl;
	}

	auto *input = window->input();
	input->on_key_event = [](const nght::KeyInfo &info)
	{
		if (info.action == nght::KeyAction::PRESS)
		{
			std::cout << "key pressed: " << static_cast<int>(info.key) << std::endl;
			if (info.key == nght::KeyCode::ESCAPE)
			{
				std::cout << "exiting...\n";
				nght::App::get().stop();
				return true;
			}
		}
		return false;
	};

	window->on_close = []()
	{
		std::cout << "Naught is closing...\n";
		nght::App::get().stop();
	};

	std::cout << "press ESC to exit\n";

	app.run();
	return 0;
}
