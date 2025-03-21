/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <vector>
#include <forge/types.hpp>
#include <forge/dev/context.hpp>
#include <forge/dev/surface.hpp>
#include <vulkan/vulkan.h>

namespace frg
{
	class Swapchain
	{
	public:
		Swapchain(const Context& ctx, const Surface& surface, const Vec2& size);
		~Swapchain();

		/* non-copyable */
		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;

		/* movable */
		Swapchain(Swapchain&&) noexcept;
		Swapchain& operator=(Swapchain&&) noexcept;

		void resize(const Vec2& size);

		[[nodiscard]] VkSwapchainKHR handle() const;
		[[nodiscard]] VkExtent2D extent() const;
		[[nodiscard]] VkFormat format() const;

		[[nodiscard]] const std::vector<VkImage>& images() const;
		[[nodiscard]] const std::vector<VkImageView>& views() const;

	private:
		void create(const Vec2& size);
		void cleanup();

		static VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
		static VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
		static VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, const Vec2& size);

		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkDevice dev = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkPhysicalDevice phys_device = VK_NULL_HANDLE;

		std::vector<VkImage> imgs;
		std::vector<VkImageView> img_views;

		VkExtent2D extnt = {};
		VkFormat img_format = VK_FORMAT_B8G8R8A8_SRGB;
		VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

		uint32_t gp_queue_family = UINT32_MAX;
		uint32_t present_queue_family = UINT32_MAX;
	};
}