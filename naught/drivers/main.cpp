/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <iostream>
#include <naught/forge/context.hpp>
#include <naught/forge/surface.hpp>
#include <naught/forge/swapchain.hpp>
#include <naught/host/app.hpp>
#include <naught/host/input.hpp>
#include <naught/host/window.hpp>

int main()
{
    auto& app = nght::App::get();

    auto* window = app.window({
        .title = "Naught",
        .bounds = { 900.0f, 700.0f },
        .style = nght::NaughtWindow::Style::STANDARD
    });

    try
    {
        nght::frg::ContextCreateInfo info;
        info.app_name = "Naught";
        info.flags = nght::frg::ContextFlags::DEFAULT;

        nght::frg::Context context(info);

        std::cout << "vulkan context created successfully!" << std::endl;
        std::cout << "using device: " << context.device() << std::endl;

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(context.physical_device(), &props);
        std::cout << "device name: " << props.deviceName << std::endl;
        std::cout << "device type: " <<
            (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? "discrete GPU" :
             props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? "integrated GPU" :
             props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU ? "cpu" : "other") << std::endl;
        std::cout << "API version: "
            << VK_VERSION_MAJOR(props.apiVersion) << "."
            << VK_VERSION_MINOR(props.apiVersion) << "."
            << VK_VERSION_PATCH(props.apiVersion) << std::endl;

        std::cout << "graphics queue family: " << context.gpq_family() << std::endl;
        std::cout << "transfer queue family: " << context.transq_family() << std::endl;
        std::cout << "compute queue family: " << context.compq_family() << std::endl;
        std::cout << "max MSAA samples: " << context.sample_count() << std::endl;

        void* native_window_handle = window->view()->native_layer();
        const nght::frg::Surface surface(context, native_window_handle);
        std::cout << "surface created successfully! handle: " << surface.handle() << std::endl;

        const nght::Vec2 size = { (window->size().first),
                          (window->size().second) };
        nght::frg::Swapchain swapchain(context, surface, size);

        std::cout << "\nswapchain created successfully!" << std::endl;
        std::cout << "swapchain handle: " << swapchain.handle() << std::endl;
        std::cout << "swapchain format: " << swapchain.format() << std::endl;
        std::cout << "swapchain extent: " << swapchain.extent().width << "x"
                 << swapchain.extent().height << std::endl;
        std::cout << "swapchain images: " << swapchain.images().size() << std::endl;
        std::cout << "swapchain image views: " << swapchain.views().size() << std::endl;

        window->on_resize = [&swapchain, window]()
        {
            std::cout << "window resized to " << window->size().first << "x" << window->size().second << "\n";
            const nght::Vec2 new_size = { (window->size().first),
                                  (window->size().second) };
            swapchain.resize(new_size);
            std::cout << "swapchain resized to " << swapchain.extent().width << "x"
                     << swapchain.extent().height << std::endl;
        };

    }
    catch (const std::exception& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }

    auto* input = window->input();
    input->on_key_event = [](const nght::KeyInfo& info)
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

    std::cout << "Press ESC to exit\n";

    app.run();
    return 0;
}