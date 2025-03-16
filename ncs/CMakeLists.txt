# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

project(ncs LANGUAGES CXX)

# library
add_library(${PROJECT_NAME}
        lib/world/world.cpp
        lib/archetype/archetypes.cpp
        lib/base/utils.cpp
        lib/storage/column.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(${PROJECT_NAME} PRIVATE
        "-O3"
        "-march=native"
)

# tests
if (NCS_BUILD_TESTS)
    find_package(GTest REQUIRED)

    set(SANITIZER_FLAGS
            "-fsanitize=address,undefined"
            "-fno-omit-frame-pointer"
            "-fno-sanitize-recover=all"
    )

    set(DEBUG_FLAGS
            "-g"
            "-Wall"
            "-Wextra"
    )

    add_executable(ncstest
            tests/archetype.cpp
            tests/crud.cpp
            tests/lifecycle.cpp
            tests/query.cpp
    )

    target_include_directories(ncstest PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_compile_options(ncstest PRIVATE
            ${SANITIZER_FLAGS}
            ${DEBUG_FLAGS}
    )

    target_link_options(ncstest PRIVATE
            ${SANITIZER_FLAGS}
    )

    target_link_libraries(ncstest PRIVATE
            ${CMAKE_PROJECT_NAME}
            GTest::GTest
            GTest::Main
    )
endif ()

if (NCS_BUILD_BENCHES)
    find_package(benchmark REQUIRED)

    set(BENCHMARKS_FLAGS
            "-O3"
            "-march=native"
            "-fno-omit-frame-pointer"
    )

    # unoptimized bench
    add_executable(ncsbm_noop benches/nooptimize.cpp)

    target_include_directories(ncsbm_noop PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(ncsbm_noop PRIVATE
            ${CMAKE_PROJECT_NAME}
            benchmark::benchmark
    )

    target_compile_options(ncsbm_noop PRIVATE
            ${BENCHMARKS_FLAGS}
    )

    # optimized bench
    add_executable(ncsbm benches/optimize.cpp)

    target_include_directories(ncsbm PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(ncsbm PRIVATE
            ${CMAKE_PROJECT_NAME}
            benchmark::benchmark
    )

    target_compile_options(ncsbm PRIVATE
            ${BENCHMARKS_FLAGS}
    )
endif ()