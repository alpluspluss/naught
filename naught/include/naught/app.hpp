/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include <string>
namespace nght
{
    class App
    {
    public:
        App();

        ~App();

        [[nodiscard]] const std::string& name() const;

        void run();

        void stop();

    private:
        std::string app_name;
        bool running;
    };
}