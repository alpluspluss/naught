/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <naught/forge/swapchain.hpp>
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace nght::frg
{
	Swapchain::Swapchain(const Context &ctx, const Surface &surface, const Vec2 &size) : dev(ctx.device()),
		surface(surface.handle()),
		phys_device(ctx.physical_device()),
		gp_queue_family(ctx.gpq_family()), present_queue_family(ctx.prq_family())
	{
		create(size);
	}

	Swapchain::~Swapchain()
	{
		cleanup();
	}

	Swapchain::Swapchain(Swapchain &&other) noexcept : swapchain(other.swapchain), dev(other.dev),
	                                                   surface(other.surface),
	                                                   phys_device(other.phys_device),
	                                                   imgs(std::move(other.imgs)),
	                                                   img_views(std::move(other.img_views)),
	                                                   extnt(other.extnt), img_format(other.img_format),
	                                                   present_mode(other.present_mode),
	                                                   gp_queue_family(other.gp_queue_family),
	                                                   present_queue_family(other.present_queue_family)
	{
		other.swapchain = VK_NULL_HANDLE;
		other.dev = VK_NULL_HANDLE;
		other.surface = VK_NULL_HANDLE;
		other.phys_device = VK_NULL_HANDLE;
	}

	Swapchain &Swapchain::operator=(Swapchain &&other) noexcept
	{
		if (this != &other)
		{
			cleanup();

			swapchain = other.swapchain;
			dev = other.dev;
			surface = other.surface;
			phys_device = other.phys_device;
			imgs = std::move(other.imgs);
			img_views = std::move(other.img_views);
			extnt = other.extnt;
			img_format = other.img_format;
			present_mode = other.present_mode;
			gp_queue_family = other.gp_queue_family;
			present_queue_family = other.present_queue_family;

			other.swapchain = VK_NULL_HANDLE;
			other.dev = VK_NULL_HANDLE;
			other.surface = VK_NULL_HANDLE;
			other.phys_device = VK_NULL_HANDLE;
		}
		return *this;
	}

	void Swapchain::resize(const Vec2 &size)
	{
		vkDeviceWaitIdle(dev); /* note: device needs to be idle before recreating the swapchain */
		cleanup();
		create(size);
	}

	VkSwapchainKHR Swapchain::handle() const
	{
		return swapchain;
	}

	VkExtent2D Swapchain::extent() const
	{
		return extnt;
	}

	VkFormat Swapchain::format() const
	{
		return img_format;
	}

	const std::vector<VkImage> &Swapchain::images() const
	{
		return imgs;
	}

	const std::vector<VkImageView> &Swapchain::views() const
	{
		return img_views;
	}

	void Swapchain::create(const Vec2 &size)
	{
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface, &capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &format_count, nullptr);

		std::vector<VkSurfaceFormatKHR> formats;
		if (format_count != 0)
		{
			formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &format_count, formats.data());
		}

		/* query present modes */
		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(phys_device, surface, &present_mode_count, nullptr);

		std::vector<VkPresentModeKHR> present_modes;
		if (present_mode_count != 0)
		{
			present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(phys_device, surface, &present_mode_count, present_modes.data());
		}

		auto [format, colorSpace] = choose_swap_surface_format(formats);
		img_format = format;

		present_mode = choose_swap_present_mode(present_modes);
		extnt = choose_swap_extent(capabilities, size);

		uint32_t image_count = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
			image_count = capabilities.maxImageCount;

		/* create swapchain */
		VkSwapchainCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = img_format;
		create_info.imageColorSpace = colorSpace;
		create_info.imageExtent = extnt;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		const uint32_t queue_family_indices[] = { gp_queue_family, present_queue_family };
		if (gp_queue_family != present_queue_family)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		create_info.preTransform = capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(dev, &create_info, nullptr, &swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain");
		}

		/* get swapchain images */
		vkGetSwapchainImagesKHR(dev, swapchain, &image_count, nullptr);
		imgs.resize(image_count);
		vkGetSwapchainImagesKHR(dev, swapchain, &image_count, imgs.data());

		/* image views */
		img_views.resize(image_count);
		for (size_t i = 0; i < image_count; i++)
		{
			VkImageViewCreateInfo view_info {};
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = imgs[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = img_format;
			view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(dev, &view_info, nullptr, &img_views[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create image views");
		}
	}

	void Swapchain::cleanup()
	{
		if (dev == VK_NULL_HANDLE)
			return;

		for (const auto view: img_views)
		{
			if (view != VK_NULL_HANDLE)
				vkDestroyImageView(dev, view, nullptr);
		}
		img_views.clear();

		if (swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(dev, swapchain, nullptr);
			swapchain = VK_NULL_HANDLE;
		}
	}

	VkSurfaceFormatKHR Swapchain::choose_swap_surface_format(
		const std::vector<VkSurfaceFormatKHR> &available_formats)
	{
		/* prefer SRGB and non-linear color space */
		for (const auto &format: available_formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		return available_formats[0]; /* just take first for now; */
	}

	VkPresentModeKHR Swapchain::choose_swap_present_mode(
		const std::vector<VkPresentModeKHR> &available_present_modes)
	{
		/* use mailbox for triple buffering */
		for (const auto &mode: available_present_modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
		}

		return VK_PRESENT_MODE_FIFO_KHR; /* fifo is guarunteed to be available */
	}

	VkExtent2D Swapchain::choose_swap_extent(
		const VkSurfaceCapabilitiesKHR &capabilities, const Vec2 &size)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;

		VkExtent2D actual_extent = {
			static_cast<uint32_t>(size.first),
			static_cast<uint32_t>(size.second)
		};

		actual_extent.width = std::clamp(actual_extent.width,
		                                 capabilities.minImageExtent.width,
		                                 capabilities.maxImageExtent.width);
		actual_extent.height = std::clamp(actual_extent.height,
		                                  capabilities.minImageExtent.height,
		                                  capabilities.maxImageExtent.height);

		return actual_extent;
	}
}
