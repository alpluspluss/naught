/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <stdexcept>
#include <iostream>
#include <set>
#include <naught/forge/context.hpp>

namespace nght::frg
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
		[[maybe_unused]] void *user_data)
	{
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			std::cerr << "Vulkan: " << callback_data->pMessage << std::endl;
		}
		return VK_FALSE;
	}

	static VkResult create_debug_utils_messenger(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT *create_info,
		const VkAllocationCallbacks *allocator,
		VkDebugUtilsMessengerEXT *debug_messenger)
	{
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr)
			return func(instance, create_info, allocator, debug_messenger);

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void destroy_debug_utils_messenger(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debug_messenger,
		const VkAllocationCallbacks *allocator)
	{
		const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func != nullptr)
		{
			func(instance, debug_messenger, allocator);
		}
	}

	Context::Context(const ContextCreateInfo &info)
	{
		if (!init(info))
			throw std::runtime_error("failed to initialize Vulkan context");
	}

	Context::~Context()
	{
		if (dev != VK_NULL_HANDLE)
		{
			vkDeviceWaitIdle(dev);
			vkDestroyDevice(dev, nullptr);
		}

		if (debug_messenger != VK_NULL_HANDLE)
		{
			destroy_debug_utils_messenger(inst, debug_messenger, nullptr);
		}

		if (inst != VK_NULL_HANDLE)
		{
			vkDestroyInstance(inst, nullptr);
		}
	}

	Context::Context(Context &&other) noexcept : inst(other.inst),
	                                             debug_messenger(other.debug_messenger),
	                                             phys_device(other.phys_device),
	                                             dev(other.dev),
	                                             graphics_queue(other.graphics_queue),
	                                             prsnt_queue(other.prsnt_queue),
	                                             trsnf_queue(other.trsnf_queue),
	                                             cp_queue(other.cp_queue),
	                                             gp_queue_family(other.gp_queue_family),
	                                             present_queue_family(other.present_queue_family),
	                                             transfer_queue_family(other.transfer_queue_family),
	                                             compute_queue_family(other.compute_queue_family),
	                                             dev_props(other.dev_props),
	                                             dev_features(other.dev_features),
	                                             max_samples(other.max_samples),
	                                             enable_validation(other.enable_validation),
	                                             validation_layers(std::move(other.validation_layers)),
	                                             dev_extensions(std::move(other.dev_extensions)),
	                                             flags(other.flags)
	{
		other.inst = VK_NULL_HANDLE;
		other.debug_messenger = VK_NULL_HANDLE;
		other.phys_device = VK_NULL_HANDLE;
		other.dev = VK_NULL_HANDLE;
		other.graphics_queue = VK_NULL_HANDLE;
		other.prsnt_queue = VK_NULL_HANDLE;
		other.trsnf_queue = VK_NULL_HANDLE;
		other.cp_queue = VK_NULL_HANDLE;
	}

	Context &Context::operator=(Context &&other) noexcept
	{
		if (this != &other)
		{
			if (dev != VK_NULL_HANDLE)
			{
				vkDeviceWaitIdle(dev);
				vkDestroyDevice(dev, nullptr);
			}

			if (debug_messenger != VK_NULL_HANDLE)
				destroy_debug_utils_messenger(inst, debug_messenger, nullptr);

			if (inst != VK_NULL_HANDLE)
				vkDestroyInstance(inst, nullptr);

			inst = other.inst;
			debug_messenger = other.debug_messenger;
			phys_device = other.phys_device;
			dev = other.dev;
			graphics_queue = other.graphics_queue;
			prsnt_queue = other.prsnt_queue;
			trsnf_queue = other.trsnf_queue;
			cp_queue = other.cp_queue;
			gp_queue_family = other.gp_queue_family;
			present_queue_family = other.present_queue_family;
			transfer_queue_family = other.transfer_queue_family;
			compute_queue_family = other.compute_queue_family;
			dev_props = other.dev_props;
			dev_features = other.dev_features;
			max_samples = other.max_samples;
			enable_validation = other.enable_validation;
			validation_layers = std::move(other.validation_layers);
			dev_extensions = std::move(other.dev_extensions);
			flags = other.flags;

			other.inst = VK_NULL_HANDLE;
			other.debug_messenger = VK_NULL_HANDLE;
			other.phys_device = VK_NULL_HANDLE;
			other.dev = VK_NULL_HANDLE;
			other.graphics_queue = VK_NULL_HANDLE;
			other.prsnt_queue = VK_NULL_HANDLE;
			other.trsnf_queue = VK_NULL_HANDLE;
			other.cp_queue = VK_NULL_HANDLE;
		}
		return *this;
	}

	VkInstance Context::instance() const
	{
		return inst;
	}

	VkPhysicalDevice Context::physical_device() const
	{
		return phys_device;
	}

	VkDevice Context::device() const
	{
		return dev;
	}

	VkQueue Context::gp_queue() const
	{
		return graphics_queue;
	}

	VkQueue Context::present_queue() const
	{
		return prsnt_queue;
	}

	VkQueue Context::transfer_queue() const
	{
		return trsnf_queue;
	}

	VkQueue Context::compute_queue() const
	{
		return cp_queue;
	}

	uint32_t Context::gpq_family() const
	{
		return gp_queue_family;
	}

	uint32_t Context::prq_family() const
	{
		return present_queue_family;
	}

	uint32_t Context::transq_family() const
	{
		return transfer_queue_family;
	}

	uint32_t Context::compq_family() const
	{
		return compute_queue_family;
	}

	bool Context::validation() const
	{
		return enable_validation;
	}

	VkSampleCountFlagBits Context::sample_count()
	{
		if (max_samples == VK_SAMPLE_COUNT_1_BIT)
			max_samples = get_max_samples();
		return max_samples;
	}

	bool Context::init(const ContextCreateInfo &info)
	{
		flags = info.flags;
		enable_validation = has_flag(flags, ContextFlags::VALIDATION);

		/* setup validation layers if enabled */
		if (enable_validation)
			validation_layers.emplace_back("VK_LAYER_KHRONOS_validation");

		/* setup required extensions */
		std::vector<const char *> extensions;
		if (has_flag(flags, ContextFlags::PRESENTATION))
			dev_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		dev_extensions.emplace_back("VK_KHR_portability_subset");

		/* create instance */
		VkApplicationInfo app_info {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = info.app_name.c_str();
		app_info.applicationVersion = info.app_version;
		app_info.pEngineName = "Naught Engine";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

		extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
		extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		extensions.emplace_back("VK_EXT_metal_surface");
		if (enable_validation)
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();
		if (enable_validation)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
		}
		else
		{
			create_info.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&create_info, nullptr, &inst) != VK_SUCCESS)
			return false;

		/* setup debug messenger if validation is enabled */
		if (enable_validation)
		{
			if (!setup_debug())
				return false;
		}

		/* pick physical device */
		if (!pick_device())
			return false;

		/* create logical device */
		if (!create_device())
			return false;

		return true;
	}

	bool Context::setup_debug()
	{
		VkDebugUtilsMessengerCreateInfoEXT create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = debug_callback;
		create_info.pUserData = nullptr;

		if (create_debug_utils_messenger(inst, &create_info, nullptr, &debug_messenger) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	bool Context::pick_device()
	{
		/* get all available physical devices */
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(inst, &device_count, nullptr);

		if (device_count == 0)
		{
			std::cerr << "No Vulkan devices found" << std::endl;
			return false;
		}

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(inst, &device_count, devices.data());

		std::cerr << "Found " << device_count << " physical devices" << std::endl;

		/* find a suitable device; preferring discrete GPUs because perf */
		int64_t best_score = -1;

		for (const auto &device: devices)
		{
			VkPhysicalDeviceProperties device_props;
			VkPhysicalDeviceFeatures device_features;

			vkGetPhysicalDeviceProperties(device, &device_props);
			vkGetPhysicalDeviceFeatures(device, &device_features);

			std::cerr << "Evaluating device: " << device_props.deviceName << std::endl;

			/* calculate score; a simple algorithm for getting the best fit */
			uint32_t score = 0;
			if (device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1000;

			/* add score based on max texture size */
			score += device_props.limits.maxImageDimension2D / 1024;

			/* check for required features - make geometryShader optional on macOS */
#ifdef __APPLE__
			if (!device_features.geometryShader)
			{
				std::cerr << "  - Warning: No geometry shader support" << std::endl;
			}
#else
        if (!device_features.geometryShader)
        {
            std::cerr << "  - Skipping: No geometry shader support" << std::endl;
            continue;
        }
#endif

			/* check for required queue families */
			uint32_t queue_family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

			std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

			/* if has required queue families */
			auto has_graphics = false;
			auto has_compute = false;
			auto has_transfer = false;
			for (uint32_t i = 0; i < queue_family_count; i++)
			{
				if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					has_graphics = true;

				if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
					has_compute = true;

				if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
					has_transfer = true;
			}

			/* skip devices that don't support the required queue families */
			auto skip = false;
			if (has_flag(flags, ContextFlags::GRAPHICS) && !has_graphics)
				skip = true;

			if (has_flag(flags, ContextFlags::COMPUTE) && !has_compute)
				skip = true;

			if (has_flag(flags, ContextFlags::TRANSFER) && !has_transfer)
				skip = true;

			if (skip)
				continue;

			if (score > best_score) /* best fit */
			{
				phys_device = device;
				best_score = score;
				dev_props = device_props;
				dev_features = device_features;
			}
		}

		if (phys_device == VK_NULL_HANDLE)
			return false;

		return true;
	}

	bool Context::create_device()
	{
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, queue_families.data());

		/* find graphics, compute, and transfer queue families */
		for (uint32_t i = 0; i < queue_family_count; i++)
		{
			if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				gp_queue_family = i;

				/* if we don't find a dedicated compute queue; use the graphics queue */
				if (compute_queue_family == UINT32_MAX)
					compute_queue_family = i;

				/* if we don't find a dedicated transfer queue; use the graphics queue */
				if (transfer_queue_family == UINT32_MAX)
					transfer_queue_family = i;
			}

			/* prefer a dedicated compute queue */
			if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT &&
			    !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				compute_queue_family = i;
			}

			/* prefer a dedicated transfer queue */
			if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
			    !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			    !(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				transfer_queue_family = i;
			}
		}

		/* if we found all required queue families */
		if (has_flag(flags, ContextFlags::GRAPHICS) && gp_queue_family == UINT32_MAX)
			return false;

		if (has_flag(flags, ContextFlags::COMPUTE) && compute_queue_family == UINT32_MAX)
			return false;

		if (has_flag(flags, ContextFlags::TRANSFER) && transfer_queue_family == UINT32_MAX)
			return false;

		/* use the graphics queue for presentation by default */
		present_queue_family = gp_queue_family;

		/* collect unique queue families */
		std::set<uint32_t> unique_queue_families;

		if (has_flag(flags, ContextFlags::GRAPHICS))
		{
			unique_queue_families.insert(gp_queue_family);
		}

		if (has_flag(flags, ContextFlags::PRESENTATION))
		{
			unique_queue_families.insert(present_queue_family);
		}

		if (has_flag(flags, ContextFlags::COMPUTE))
		{
			unique_queue_families.insert(compute_queue_family);
		}

		if (has_flag(flags, ContextFlags::TRANSFER))
		{
			unique_queue_families.insert(transfer_queue_family);
		}

		/* create one queue from each family */
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		auto queue_priority = 1.0f;

		for (uint32_t queue_family: unique_queue_families)
		{
			VkDeviceQueueCreateInfo queue_create_info {};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.emplace_back(queue_create_info);
		}

		/* specify the device features we need */
		VkPhysicalDeviceFeatures device_features {};
		device_features.samplerAnisotropy = VK_TRUE;

		/* create the logical device */
		VkDeviceCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.pQueueCreateInfos = queue_create_infos.data();
		create_info.pEnabledFeatures = &device_features;
		create_info.enabledExtensionCount = static_cast<uint32_t>(dev_extensions.size());
		create_info.ppEnabledExtensionNames = dev_extensions.data();

		/* legacy compatibility with older Vulkan implementations */
		if (enable_validation)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
		}
		else
		{
			create_info.enabledLayerCount = 0;
		}

		/* create the logical device */
		if (vkCreateDevice(phys_device, &create_info, nullptr, &dev) != VK_SUCCESS)
			return false;

		/* get queue handles */
		if (has_flag(flags, ContextFlags::GRAPHICS))
			vkGetDeviceQueue(dev, gp_queue_family, 0, &graphics_queue);

		if (has_flag(flags, ContextFlags::PRESENTATION))
			vkGetDeviceQueue(dev, present_queue_family, 0, &prsnt_queue);

		if (has_flag(flags, ContextFlags::COMPUTE))
			vkGetDeviceQueue(dev, compute_queue_family, 0, &cp_queue);

		if (has_flag(flags, ContextFlags::TRANSFER))
			vkGetDeviceQueue(dev, transfer_queue_family, 0, &trsnf_queue);

		return true;
	}

	VkSampleCountFlagBits Context::get_max_samples()
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(phys_device, &props);

		VkSampleCountFlags counts = props.limits.framebufferColorSampleCounts &
		                            props.limits.framebufferDepthSampleCounts;

		if (counts & VK_SAMPLE_COUNT_64_BIT)
			return VK_SAMPLE_COUNT_64_BIT;
		if (counts & VK_SAMPLE_COUNT_32_BIT)
			return VK_SAMPLE_COUNT_32_BIT;
		if (counts & VK_SAMPLE_COUNT_16_BIT)
			return VK_SAMPLE_COUNT_16_BIT;
		if (counts & VK_SAMPLE_COUNT_8_BIT)
			return VK_SAMPLE_COUNT_8_BIT;
		if (counts & VK_SAMPLE_COUNT_4_BIT)
			return VK_SAMPLE_COUNT_4_BIT;
		if (counts & VK_SAMPLE_COUNT_2_BIT)
			return VK_SAMPLE_COUNT_2_BIT;

		return VK_SAMPLE_COUNT_1_BIT;
	}
}
