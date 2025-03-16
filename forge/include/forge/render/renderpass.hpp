/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <vector>
#include <forge/dev/context.hpp>
#include <vulkan/vulkan.h>

namespace frg
{
    enum class AttachmentLoadOp
    {
        LOAD = VK_ATTACHMENT_LOAD_OP_LOAD,
        CLEAR = VK_ATTACHMENT_LOAD_OP_CLEAR,
        DONT_CARE = VK_ATTACHMENT_LOAD_OP_DONT_CARE
    };

    enum class AttachmentStoreOp
    {
        STORE = VK_ATTACHMENT_STORE_OP_STORE,
        DONT_CARE = VK_ATTACHMENT_STORE_OP_DONT_CARE
    };

    enum class ImageLayout
    {
        UNDEFINED = VK_IMAGE_LAYOUT_UNDEFINED,
        GENERAL = VK_IMAGE_LAYOUT_GENERAL,
        COLOR_ATTACHMENT = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        DEPTH_STENCIL_READ_ONLY = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        SHADER_READ_ONLY = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        TRANSFER_SRC = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        TRANSFER_DST = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        PREINITIALIZED = VK_IMAGE_LAYOUT_PREINITIALIZED,
        PRESENT_SRC = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    struct AttachmentDescription
    {
        VkFormat format = VK_FORMAT_UNDEFINED;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        AttachmentLoadOp load_op = AttachmentLoadOp::CLEAR;
        AttachmentStoreOp store_op = AttachmentStoreOp::STORE;
        AttachmentLoadOp stencil_load_op = AttachmentLoadOp::DONT_CARE;
        AttachmentStoreOp stencil_store_op = AttachmentStoreOp::DONT_CARE;
        ImageLayout initial_layout = ImageLayout::UNDEFINED;
        ImageLayout final_layout = ImageLayout::PRESENT_SRC;
    };

    struct AttachmentReference
    {
        uint32_t attachment;
        ImageLayout layout;
    };

    struct SubpassDescription
    {
        VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
        std::vector<AttachmentReference> input_attachments;
        std::vector<AttachmentReference> color_attachments;
        std::vector<AttachmentReference> resolve_attachments;
        AttachmentReference depth_stencil_attachment = { UINT32_MAX, ImageLayout::UNDEFINED };
        std::vector<uint32_t> preserve_attachments;
    };

    struct SubpassDependency
    {
        uint32_t src_subpass;
        uint32_t dst_subpass;
        VkPipelineStageFlags src_stage_mask;
        VkPipelineStageFlags dst_stage_mask;
        VkAccessFlags src_access_mask;
        VkAccessFlags dst_access_mask;
        VkDependencyFlags dependency_flags = 0;
    };

    class RenderPass
    {
    public:
        RenderPass(Context& ctx,
                  const std::vector<AttachmentDescription>& attachments,
                  const std::vector<SubpassDescription>& subpasses,
                  const std::vector<SubpassDependency>& dependencies = {});
        ~RenderPass();

        /* non-copyable */
        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;

        /* movable */
        RenderPass(RenderPass&&) noexcept;
        RenderPass& operator=(RenderPass&&) noexcept;

        [[nodiscard]] VkRenderPass handle() const;

        /* helper to create a simple single-subpass render pass */
        static RenderPass create_simple(
            Context& ctx,
            VkFormat color_format,
            VkFormat depth_format = VK_FORMAT_UNDEFINED,
            VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

    private:
        VkRenderPass render_pass = VK_NULL_HANDLE;
        VkDevice dev = VK_NULL_HANDLE;
    };
}