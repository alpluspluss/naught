# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

project(forge LANGUAGES CXX)

# deps resolution & compiling
find_package(Vulkan REQUIRED)

# deps: vulkan memory alloc
add_library(vkma STATIC ext/vkma/lib/vkma.cpp)
target_include_directories(vkma PRIVATE ext/vkma/include)
target_link_libraries(vkma PRIVATE Vulkan::Vulkan)

# main lib
add_library(${PROJECT_NAME}
        lib/buf/buffer.cpp lib/buf/ib.cpp lib/buf/ub.cpp lib/buf/vb.cpp
        lib/dev/context.cpp lib/dev/surface.cpp
        lib/swapchain/swapchain.cpp
        lib/sync/barrier.cpp lib/sync/fence.cpp lib/sync/semaphore.cpp
        lib/cmd/buffer.cpp lib/cmd/manager.cpp lib/cmd/pool.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

        # deps dirs
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/vkma/include
        ${Vulkan_INCLUDE_DIRS}
)

# tests
if (FORGE_BUILD_TESTS)
    find_package(GTest REQUIRED)

    set(SANITIZER_FLAGS "-fsanitize=address,undefined" "-fno-omit-frame-pointer" "-fno-sanitize-recover=all")
    set(DEBUG_FLAGS "-g" "-Wall" "-Wextra")

    add_executable(forgetest
            tests/sync/barrier.cpp tests/sync/fence.cpp tests/sync/semaphore.cpp
            tests/buf/buffer.cpp
            tests/dev/context.cpp
            tests/cmd/cmd.cpp
    )

    target_include_directories(forgetest PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/naught/include

            # deps dirs
            ${CMAKE_CURRENT_SOURCE_DIR}/ext/vkma/include
            ${Vulkan_INCLUDE_DIRS}
    )

    target_link_libraries(forgetest PRIVATE
            ${PROJECT_NAME} vkma Vulkan::Vulkan GTest::GTest GTest::Main naught
    )

    target_link_options(forgetest PRIVATE ${SANITIZER_FLAGS})
    target_compile_options(forgetest PRIVATE ${SANITIZER_FLAGS} ${DEBUG_FLAGS})
endif ()

# bench: later