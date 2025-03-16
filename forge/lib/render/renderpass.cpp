/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <stdexcept>
#include <forge/render/renderpass.hpp>

namespace frg
{
	RenderPass::RenderPass(Context &ctx,
	                       const std::vector<AttachmentDescription> &attachments,
	                       const std::vector<SubpassDescription> &subpasses,
	                       const std::vector<SubpassDependency> &dependencies) : dev(ctx.device())
	{
		std::vector<VkAttachmentDescription> vk_attachments;
		vk_attachments.reserve(attachments.size());

		for (const auto &att: attachments)
		{
			VkAttachmentDescription vk_att = {};
			vk_att.format = att.format;
			vk_att.samples = att.samples;
			vk_att.loadOp = static_cast<VkAttachmentLoadOp>(att.load_op);
			vk_att.storeOp = static_cast<VkAttachmentStoreOp>(att.store_op);
			vk_att.stencilLoadOp = static_cast<VkAttachmentLoadOp>(att.stencil_load_op);
			vk_att.stencilStoreOp = static_cast<VkAttachmentStoreOp>(att.stencil_store_op);
			vk_att.initialLayout = static_cast<VkImageLayout>(att.initial_layout);
			vk_att.finalLayout = static_cast<VkImageLayout>(att.final_layout);
			vk_attachments.emplace_back(vk_att);
		}

		/* IMPORTANT: These vectors need to live for the duration of this function call */
		std::vector<std::vector<VkAttachmentReference> > color_refs(subpasses.size());
		std::vector<std::vector<VkAttachmentReference> > input_refs(subpasses.size());
		std::vector<std::vector<VkAttachmentReference> > resolve_refs(subpasses.size());
		std::vector<std::vector<uint32_t> > preserve_refs(subpasses.size());
		std::vector<VkAttachmentReference> depth_refs(subpasses.size());

		for (size_t i = 0; i < subpasses.size(); i++)
			depth_refs[i] = { UINT32_MAX, VK_IMAGE_LAYOUT_UNDEFINED };

		std::vector<VkSubpassDescription> vk_subpasses;
		vk_subpasses.reserve(subpasses.size());

		for (size_t i = 0; i < subpasses.size(); ++i)
		{
			const auto &[bind_point, input_attachments, color_attachments, resolve_attachments, depth_stencil_attachment
				, preserve_attachments] = subpasses[i];

			color_refs[i].reserve(color_attachments.size());
			for (const auto &[attachment, layout]: color_attachments)
			{
				VkAttachmentReference ref = {};
				ref.attachment = attachment;
				ref.layout = static_cast<VkImageLayout>(layout);
				color_refs[i].emplace_back(ref);
			}

			if (depth_stencil_attachment.attachment != UINT32_MAX)
			{
				VkAttachmentReference ref = {};
				ref.attachment = depth_stencil_attachment.attachment;
				ref.layout = static_cast<VkImageLayout>(depth_stencil_attachment.layout);
				depth_refs[i] = ref;
			}

			input_refs[i].reserve(input_attachments.size());
			for (const auto &[attachment, layout]: input_attachments)
			{
				VkAttachmentReference ref = {};
				ref.attachment = attachment;
				ref.layout = static_cast<VkImageLayout>(layout);
				input_refs[i].emplace_back(ref);
			}

			resolve_refs[i].reserve(resolve_attachments.size());
			for (const auto &resolve_att: resolve_attachments)
			{
				VkAttachmentReference ref = {};
				ref.attachment = resolve_att.attachment;
				ref.layout = static_cast<VkImageLayout>(resolve_att.layout);
				resolve_refs[i].emplace_back(ref);
			}

			preserve_refs[i] = preserve_attachments;

			VkSubpassDescription vk_subpass = {};
			vk_subpass.pipelineBindPoint = bind_point;

			if (!color_refs[i].empty())
			{
				vk_subpass.colorAttachmentCount = static_cast<uint32_t>(color_refs[i].size());
				vk_subpass.pColorAttachments = color_refs[i].data();
			}

			if (depth_stencil_attachment.attachment != UINT32_MAX)
				vk_subpass.pDepthStencilAttachment = &depth_refs[i];

			if (!input_refs[i].empty())
			{
				vk_subpass.inputAttachmentCount = static_cast<uint32_t>(input_refs[i].size());
				vk_subpass.pInputAttachments = input_refs[i].data();
			}

			if (!resolve_refs[i].empty() && resolve_refs[i].size() == color_refs[i].size())
			{
				vk_subpass.pResolveAttachments = resolve_refs[i].data();
			}

			if (!preserve_refs[i].empty())
			{
				vk_subpass.preserveAttachmentCount = static_cast<uint32_t>(preserve_refs[i].size());
				vk_subpass.pPreserveAttachments = preserve_refs[i].data();
			}

			vk_subpasses.emplace_back(vk_subpass);
		}

		std::vector<VkSubpassDependency> vk_dependencies;
		vk_dependencies.reserve(dependencies.size());

		for (const auto &dep: dependencies)
		{
			VkSubpassDependency vk_dep = {};
			vk_dep.srcSubpass = dep.src_subpass;
			vk_dep.dstSubpass = dep.dst_subpass;
			vk_dep.srcStageMask = dep.src_stage_mask;
			vk_dep.dstStageMask = dep.dst_stage_mask;
			vk_dep.srcAccessMask = dep.src_access_mask;
			vk_dep.dstAccessMask = dep.dst_access_mask;
			vk_dep.dependencyFlags = dep.dependency_flags;

			vk_dependencies.emplace_back(vk_dep);
		}

		VkRenderPassCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.attachmentCount = static_cast<uint32_t>(vk_attachments.size());
		create_info.pAttachments = vk_attachments.data();
		create_info.subpassCount = static_cast<uint32_t>(vk_subpasses.size());
		create_info.pSubpasses = vk_subpasses.data();

		if (!vk_dependencies.empty())
		{
			create_info.dependencyCount = static_cast<uint32_t>(vk_dependencies.size());
			create_info.pDependencies = vk_dependencies.data();
		}

		if (vkCreateRenderPass(dev, &create_info, nullptr, &render_pass) != VK_SUCCESS)
			throw std::runtime_error("failed to create render pass");
	}

	RenderPass::~RenderPass()
	{
		if (render_pass != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(dev, render_pass, nullptr);
			render_pass = VK_NULL_HANDLE;
		}
	}

	RenderPass::RenderPass(RenderPass &&other) noexcept : render_pass(other.render_pass), dev(other.dev)
	{
		other.render_pass = VK_NULL_HANDLE;
		other.dev = VK_NULL_HANDLE;
	}

	RenderPass &RenderPass::operator=(RenderPass &&other) noexcept
	{
		if (this != &other)
		{
			if (render_pass != VK_NULL_HANDLE && dev != VK_NULL_HANDLE)
				vkDestroyRenderPass(dev, render_pass, nullptr);

			render_pass = other.render_pass;
			dev = other.dev;

			other.render_pass = VK_NULL_HANDLE;
			other.dev = VK_NULL_HANDLE;
		}
		return *this;
	}

	VkRenderPass RenderPass::handle() const
	{
		return render_pass;
	}

	RenderPass RenderPass::create_simple(
		Context &ctx,
		const VkFormat color_format,
		const VkFormat depth_format,
		const VkSampleCountFlagBits samples)
	{
		std::vector<AttachmentDescription> attachments;

		AttachmentDescription color_attachment = {};
		color_attachment.format = color_format;
		color_attachment.samples = samples;
		color_attachment.load_op = AttachmentLoadOp::CLEAR;
		color_attachment.store_op = AttachmentStoreOp::STORE;
		color_attachment.stencil_load_op = AttachmentLoadOp::DONT_CARE;
		color_attachment.stencil_store_op = AttachmentStoreOp::DONT_CARE;
		color_attachment.initial_layout = ImageLayout::UNDEFINED;
		color_attachment.final_layout = ImageLayout::PRESENT_SRC;
		attachments.emplace_back(color_attachment);

		SubpassDescription subpass = {};
		subpass.bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;

		AttachmentReference color_ref = {};
		color_ref.attachment = 0;
		color_ref.layout = ImageLayout::COLOR_ATTACHMENT;
		subpass.color_attachments.emplace_back(color_ref);

		/* add depth attachment given the format is provided */
		if (depth_format != VK_FORMAT_UNDEFINED)
		{
			AttachmentDescription depth_attachment = {};
			depth_attachment.format = depth_format;
			depth_attachment.samples = samples;
			depth_attachment.load_op = AttachmentLoadOp::CLEAR;
			depth_attachment.store_op = AttachmentStoreOp::DONT_CARE;
			depth_attachment.stencil_load_op = AttachmentLoadOp::DONT_CARE;
			depth_attachment.stencil_store_op = AttachmentStoreOp::DONT_CARE;
			depth_attachment.initial_layout = ImageLayout::UNDEFINED;
			depth_attachment.final_layout = ImageLayout::DEPTH_STENCIL_ATTACHMENT;
			attachments.emplace_back(depth_attachment);

			/* depth attachment reference */
			AttachmentReference depth_ref = {};
			depth_ref.attachment = 1;
			depth_ref.layout = ImageLayout::DEPTH_STENCIL_ATTACHMENT;
			subpass.depth_stencil_attachment = depth_ref;
		}

		/* create a dependency to ensure proper synchronization */
		SubpassDependency dependency = {};
		dependency.src_subpass = VK_SUBPASS_EXTERNAL;
		dependency.dst_subpass = 0;
		dependency.src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.src_access_mask = 0;
		dependency.dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		/* add depth dependency if using depth */
		if (depth_format != VK_FORMAT_UNDEFINED)
		{
			dependency.src_stage_mask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dst_stage_mask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dst_access_mask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		return RenderPass(ctx, attachments, { subpass }, { dependency });
	}
}
