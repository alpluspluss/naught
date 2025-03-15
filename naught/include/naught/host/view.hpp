/* this file is a part of Naught Engine which is under MIT license; see LICENSE for more info */

#pragma once

#include<memory>
#include <functional>
#include <naught/types.hpp>

namespace nght 
{
    class NaughtWindow;

    class View
    {
    public:
        View(void* handle);

        ~View();

        Vec2 size() const;

        void clear(float r, float g, float b, float a = 1.0f);

        /* render */
        bool begin();

        void end();

        /* native shit */
        void* native_view() const;

        void* native_layer() const;

        void* native_device() const;

        void* native_cmdq() const;

        /* callback */
        std::function<void()> on_render;

        std::function<void(Vec2)> on_resize;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };
}