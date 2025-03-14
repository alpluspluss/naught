/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#include <iostream>
#include <naught/app.hpp>
#include <naught/types.hpp>
#include <naught/window.hpp>

int main()
{
    nght::App app;
    nght::NaughtWindow window("Naught", nght::NaughtWindow::Style::STANDARD, nght::Vec2(900, 700));

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

    app.run();
    return 0;
}