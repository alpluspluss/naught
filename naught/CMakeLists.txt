# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

# deps resolution & compiling
find_package(Vulkan REQUIRED)

# vulkan memory alloc
add_library(vkma STATIC
        ext/vkma/src/vkma.cpp
)

target_include_directories(vkma PRIVATE
        ext/vkma/include
)

target_link_libraries(vkma PRIVATE
        Vulkan::Vulkan
)

# naught lib
set(NAUGHT_SOURCE
        lib/forge/context.cpp
        lib/forge/buffer.cpp
        lib/forge/ib.cpp
        lib/forge/surface.cpp
        lib/forge/swapchain.cpp
        lib/forge/ub.cpp
        lib/forge/vb.cpp
)

if (APPLE)
    list(APPEND NAUGHT_SOURCE
            lib/host/macos/window.mm
            lib/host/macos/app.mm
            lib/host/macos/view.mm
            lib/host/macos/input.mm
    )
endif ()

add_library(naught
        ${NAUGHT_SOURCE}
)

target_include_directories(naught PRIVATE
        ${PROJECT_SOURCE_DIR}/naught/include
        ${PROJECT_SOURCE_DIR}/necs/include

        # ext deps
        ext/vkma/include
)

if (VULKAN_FOUND)
    message(STATUS "Found Vulkan. Including and linking...")
    target_link_libraries(naught PRIVATE
            Vulkan::Vulkan
            vkma
    )
    target_include_directories(naught PRIVATE ${Vulkan_INCLUDE_DIRS})
endif ()

if (APPLE)
    target_link_libraries(naught PRIVATE
            "-framework AppKit"
            "-framework Cocoa"
            "-framework Metal"
            "-framework QuartzCore"
    )
endif ()


# driver
add_executable(Naught
        drivers/main.cpp
)

target_include_directories(Naught PRIVATE
        ${PROJECT_SOURCE_DIR}/naught/include
        ${PROJECT_SOURCE_DIR}/necs/include

        # ext deps
        ext/vkma/include
)

target_link_libraries(Naught PRIVATE
        necs
        naught
        vkma
)

if (VULKAN_FOUND)
    target_link_libraries(Naught PRIVATE
            Vulkan::Vulkan
    )
    target_include_directories(Naught PRIVATE ${Vulkan_INCLUDE_DIRS})
endif ()