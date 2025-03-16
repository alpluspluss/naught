# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

project(forge LANGUAGES CXX)

# deps resolution & compiling
find_package(Vulkan REQUIRED)

# deps: vulkan memory alloc
add_library(vkma STATIC
        ext/vkma/lib/vkma.cpp
)

target_include_directories(vkma PRIVATE
        ext/vkma/include
)

# main lib
add_library(${PROJECT_NAME}
        lib/buf/buffer.cpp
        lib/buf/ib.cpp
        lib/buf/ub.cpp
        lib/buf/vb.cpp

        lib/dev/context.cpp
        lib/dev/surface.cpp

        lib/swapchain/swapchain.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

        # deps dirs
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/vkma/include
)

if (VULKAN_FOUND)
    message(STATUS "Found Vulkan. Including and linking...")
    target_link_libraries(vkma PRIVATE
            Vulkan::Vulkan
    )

    target_link_libraries(${PROJECT_NAME} PUBLIC
            vkma
            Vulkan::Vulkan
    )

    target_include_directories(vkma PRIVATE ${Vulkan_INCLUDE_DIRS})

    target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
endif ()