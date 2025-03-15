/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace nght::frg
{
   enum class ContextFlags
   {
      NONE = 0,
      VALIDATION = 1 << 0,
      PRESENTATION = 1 << 1,
      COMPUTE = 1 << 2,
      TRANSFER = 1 << 3,
      GRAPHICS = 1 << 4,
      DEFAULT = VALIDATION | PRESENTATION | GRAPHICS
   };

   /* bitwise operators for ContextFlags */
   inline ContextFlags operator|(ContextFlags a, ContextFlags b)
   {
      return static_cast<ContextFlags>(
         static_cast<int>(a) | static_cast<int>(b)
      );
   }

   inline bool has_flag(ContextFlags flags, ContextFlags flag)
   {
      return (static_cast<int>(flags) & static_cast<int>(flag)) != 0;
   }

   struct ContextCreateInfo
   {
      std::string app_name = "Naught";
      uint32_t app_version = VK_MAKE_VERSION(1, 0, 0);
      ContextFlags flags = ContextFlags::DEFAULT;
   };

   class Context
   {
   public:
      explicit Context(const ContextCreateInfo &info);

      ~Context();

      /* non-copyable */
      Context(const Context &) = delete;

      Context &operator=(const Context &) = delete;

      /* movable */
      Context(Context &&) noexcept;

      Context &operator=(Context &&) noexcept;

      /* getters */
      [[nodiscard]] VkInstance instance() const;

      [[nodiscard]] VkPhysicalDevice physical_device() const;

      [[nodiscard]] VkDevice device() const;

      [[nodiscard]] VkQueue gp_queue() const;

      [[nodiscard]] VkQueue present_queue() const;

      [[nodiscard]] VkQueue transfer_queue() const;

      [[nodiscard]] VkQueue compute_queue() const;

      [[nodiscard]] uint32_t gpq_family() const;

      [[nodiscard]] uint32_t prq_family() const;

      [[nodiscard]] uint32_t transq_family() const;

      [[nodiscard]] uint32_t compq_family() const;

      [[nodiscard]] VkSampleCountFlagBits sample_count();

      [[nodiscard]] bool validation() const;

   private:
      bool init(const ContextCreateInfo &info);

      bool setup_debug();

      bool pick_device();

      bool create_device();

      VkSampleCountFlagBits get_max_samples();

      /* vkobjects */
      VkInstance inst = VK_NULL_HANDLE;
      VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
      VkPhysicalDevice phys_device = VK_NULL_HANDLE;
      VkDevice dev = VK_NULL_HANDLE;

      /* queue handles */
      VkQueue graphics_queue = VK_NULL_HANDLE;
      VkQueue prsnt_queue = VK_NULL_HANDLE;
      VkQueue trsnf_queue = VK_NULL_HANDLE;
      VkQueue cp_queue = VK_NULL_HANDLE;

      /* queue family indices */
      uint32_t gp_queue_family = UINT32_MAX;
      uint32_t present_queue_family = UINT32_MAX;
      uint32_t transfer_queue_family = UINT32_MAX;
      uint32_t compute_queue_family = UINT32_MAX;

      /* device features */
      VkPhysicalDeviceProperties dev_props = {};
      VkPhysicalDeviceFeatures dev_features = {};
      VkSampleCountFlagBits max_samples = VK_SAMPLE_COUNT_1_BIT;

      /* config */
      bool enable_validation = false;
      std::vector<const char *> validation_layers;
      std::vector<const char *> dev_extensions;
      ContextFlags flags = ContextFlags::DEFAULT;
   };
}
