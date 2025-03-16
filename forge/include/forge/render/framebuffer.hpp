/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <vector>
#include <forge/dev/context.hpp>
#include <forge/render/renderpass.hpp>
#include <vulkan/vulkan.h>

namespace frg
{
	class Framebuffer
	{
	public:
		Framebuffer(const Context& ctx,
				   const RenderPass& render_pass,
				   const std::vector<VkImageView>& attachments,
				   uint32_t width,
				   uint32_t height,
				   uint32_t layers = 1);
		~Framebuffer();

		/* non-copyable */
		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;

		/* movable */
		Framebuffer(Framebuffer&&) noexcept;
		Framebuffer& operator=(Framebuffer&&) noexcept;

		[[nodiscard]] VkFramebuffer handle() const;
		[[nodiscard]] uint32_t width() const;
		[[nodiscard]] uint32_t height() const;
		[[nodiscard]] uint32_t layers() const;

	private:
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VkDevice dev = VK_NULL_HANDLE;
		uint32_t w = 0;
		uint32_t h = 0;
		uint32_t l = 1;
	};
}