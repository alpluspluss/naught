/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <naught/forge/surface.hpp>
#if defined(__APPLE__)
#include <vulkan/vulkan_metal.h>
#elif defined(_WIN32)
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#else
#include <vulkan/vulkan_xlib.h>
#endif

namespace nght::frg
{
	Surface::Surface(const Context& ctx, void* window_handle) : instance(ctx.instance())
	{
#if defined(__APPLE__)
		VkMetalSurfaceCreateInfoEXT create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
		create_info.pLayer = window_handle;

		if (vkCreateMetalSurfaceEXT(instance, &create_info, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create Metal surface");
#elif defined(_WIN32)
		VkWin32SurfaceCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create_info.hwnd = static_cast<HWND>(window_handle);
		create_info.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create Win32 surface");
#else
		VkXlibSurfaceCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		create_info.dpy = XOpenDisplay(nullptr);
		create_info.window = *static_cast<Window*>(window_handle);

		if (vkCreateXlibSurfaceKHR(instance, &create_info, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create Xlib surface");
#endif
	}

	Surface::Surface(Surface&& other) noexcept
		: surface(other.surface), instance(other.instance)
	{
		other.surface = VK_NULL_HANDLE;
		other.instance = VK_NULL_HANDLE;
	}

	Surface::~Surface()
	{
		if (surface != VK_NULL_HANDLE && instance != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = VK_NULL_HANDLE;
		}
	}

	Surface& Surface::operator=(Surface&& other) noexcept
	{
		if (this != &other)
		{
			if (surface != VK_NULL_HANDLE && instance != VK_NULL_HANDLE)
				vkDestroySurfaceKHR(instance, surface, nullptr);

			surface = other.surface;
			instance = other.instance;

			other.surface = VK_NULL_HANDLE;
			other.instance = VK_NULL_HANDLE;
		}
		return *this;
	}

	VkSurfaceKHR Surface::handle() const
	{
		return surface;
	}
}