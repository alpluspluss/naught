# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

# naught lib

find_package(Vulkan REQUIRED)

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
)

if (VULKAN_FOUND)
    message(STATUS "Found Vulkan. Including and linking...")
    target_link_libraries(naught PRIVATE
            Vulkan::Vulkan
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
)

target_link_libraries(Naught PRIVATE
        necs
        naught
)

if (VULKAN_FOUND)
    message(STATUS "Found Vulkan. Including and linking...")
    target_link_libraries(Naught PRIVATE
            Vulkan::Vulkan
    )
    target_include_directories(Naught PRIVATE ${Vulkan_INCLUDE_DIRS})
endif ()