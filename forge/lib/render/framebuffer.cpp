/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <stdexcept>
#include <forge/render/framebuffer.hpp>

namespace frg
{
    Framebuffer::Framebuffer(const Context& ctx,
                           const RenderPass& render_pass,
                           const std::vector<VkImageView>& attachments,
                           const uint32_t width,
                           const uint32_t height,
                           const uint32_t layers)
        : dev(ctx.device()), w(width), h(height), l(layers)
    {
        VkFramebufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = render_pass.handle();
        create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        create_info.pAttachments = attachments.data();
        create_info.width = width;
        create_info.height = height;
        create_info.layers = layers;

        if (vkCreateFramebuffer(dev, &create_info, nullptr, &framebuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }

    Framebuffer::~Framebuffer()
    {
        if (framebuffer != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(dev, framebuffer, nullptr);
            framebuffer = VK_NULL_HANDLE;
        }
    }

    Framebuffer::Framebuffer(Framebuffer&& other) noexcept
        : framebuffer(other.framebuffer), dev(other.dev), w(other.w), h(other.h), l(other.l)
    {
        other.framebuffer = VK_NULL_HANDLE;
        other.dev = VK_NULL_HANDLE;
        other.w = 0;
        other.h = 0;
        other.l = 0;
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
    {
        if (this != &other)
        {
            if (framebuffer != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
                vkDestroyFramebuffer(dev, framebuffer, nullptr);

            framebuffer = other.framebuffer;
            dev = other.dev;
            w = other.w;
            h = other.h;
            l = other.l;

            other.framebuffer = VK_NULL_HANDLE;
            other.dev = VK_NULL_HANDLE;
            other.w = 0;
            other.h = 0;
            other.l = 0;
        }
        return *this;
    }

    VkFramebuffer Framebuffer::handle() const
    {
        return framebuffer;
    }

    uint32_t Framebuffer::width() const
    {
        return w;
    }

    uint32_t Framebuffer::height() const
    {
        return h;
    }

    uint32_t Framebuffer::layers() const
    {
        return l;
    }
}