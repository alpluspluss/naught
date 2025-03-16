# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

find_package(Vulkan REQUIRED)

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
        ${PROJECT_SOURCE_DIR}/ncs/include
)

if (APPLE)
    target_link_libraries(naught PRIVATE
            "-framework AppKit"
            "-framework Cocoa"
            "-framework Metal"
            "-framework QuartzCore"
    )
endif ()

add_executable(Naught
        drivers/main.cpp
)

target_include_directories(Naught PRIVATE
        ${PROJECT_SOURCE_DIR}/forge/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include

        ${PROJECT_SOURCE_DIR}/forge/ext/vkma/include
        ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Naught PRIVATE
        forge
        ncs
        naught
        vkma
        Vulkan::Vulkan
)