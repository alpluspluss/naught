/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <iostream>
#include <naught/forge/context.hpp>
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
        info.app_name = "Naught Vulkan";
        info.flags = nght::frg::ContextFlags::DEFAULT;

        nght::frg::Context context(info);

        std::cout << "Vulkan context created successfully!" << std::endl;
        std::cout << "Using device: " << context.device() << std::endl;

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(context.physical_device(), &props);
        std::cout << "Device name: " << props.deviceName << std::endl;
        std::cout << "Device type: " <<
            (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? "Discrete GPU" :
             props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? "Integrated GPU" :
             props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU ? "CPU" : "Other") << std::endl;
        std::cout << "API version: "
            << VK_VERSION_MAJOR(props.apiVersion) << "."
            << VK_VERSION_MINOR(props.apiVersion) << "."
            << VK_VERSION_PATCH(props.apiVersion) << std::endl;

        std::cout << "Graphics queue family: " << context.gpq_family() << std::endl;
        std::cout << "Transfer queue family: " << context.transq_family() << std::endl;
        std::cout << "Compute queue family: " << context.compq_family() << std::endl;
        std::cout << "Max MSAA samples: " << context.sample_count() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    auto* input = window->input();
    input->on_key_event = [](const nght::KeyInfo& info)
    {
        if (info.action == nght::KeyAction::PRESS)
        {
            std::cout << "Key pressed: " << static_cast<int>(info.key) << std::endl;
            if (info.key == nght::KeyCode::ESCAPE)
            {
                std::cout << "Escape pressed, exiting...\n";
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

    window->on_resize = [window]()
    {
        std::cout << "Resized to " << window->size().first << "x" << window->size().second << "\n";
    };

    std::cout << "Press ESC to exit\n";

    app.run();
    return 0;
}