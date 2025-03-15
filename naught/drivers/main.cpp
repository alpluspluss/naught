/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <iostream>
#include <naught/types.hpp>
#include <naught/host/app.hpp>
#include <naught/host/input.hpp>
#include <naught/host/window.hpp>

int main()
{
    nght::App app;
    nght::NaughtWindow window("Naught Input Test", nght::NaughtWindow::Style::STANDARD, nght::Vec2(900, 700));
    auto *input = window.create_input();
    input->on_key_event = [&app](const nght::KeyInfo &info)
    {
        if (info.action == nght::KeyAction::PRESS)
        {
            std::cout << "Key pressed: " << static_cast<int>(info.key) << std::endl;
            if (info.key == nght::KeyCode::ESCAPE)
            {
                std::cout << "Escape pressed, exiting...\n";
                app.stop();
                return true;
            }
        }
        return false;
    };

    input->on_mouse_move = [](const nght::Vec2 &pos, const nght::Vec2 &delta)
    {
        std::cout << "Mouse at: " << pos.first << ", " << pos.second << std::endl;
    };

    input->on_mouse_button = [](int button, bool pressed, int mods)
    {
        if (pressed)
        {
            std::cout << "Mouse button " << button << " pressed" << std::endl;
        }
        else
        {
            std::cout << "Mouse button " << button << " released" << std::endl;
        }
    };

    window.on_close = [&app]()
    {
        std::cout << "Naught Window is closing...\n";
        app.stop();
    };

    window.on_resize = [&window]()
    {
        std::cout << "Resized to " << window.size().first << "x" << window.size().second << "\n";
    };

    window.size(nght::Vec2(800, 600));

    auto [fst, snd] = window.size();
    window.position(nght::Vec2(1920 / 2 - fst / 2, 1080 / 2 - snd / 2));

    std::cout << "Press any key, move mouse, or click mouse buttons\n";
    std::cout << "Press ESC to exit\n";

    app.run();
    return 0;
}
