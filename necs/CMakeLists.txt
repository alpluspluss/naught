# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

# library
add_library(necs
        lib/world.cpp
        lib/archetypes.cpp
        lib/utils.cpp
        lib/column.cpp
)

target_include_directories(necs PRIVATE
        include
)

# tests
if (NECS_BUILD_TESTS)
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

    add_executable(necstest
            tests/archetype.cpp
            tests/crud.cpp
            tests/lifecycle.cpp
            tests/query.cpp
    )

    target_include_directories(necstest PRIVATE
            ${PROJECT_SOURCE_DIR}/necs/include
    )

    target_compile_options(necstest PRIVATE
            ${SANITIZER_FLAGS}
            ${DEBUG_FLAGS}
    )

    target_link_options(necstest PRIVATE
            ${SANITIZER_FLAGS}
    )

    target_link_libraries(necstest PRIVATE
            necs
            GTest::GTest
            GTest::Main
    )
endif ()

if (NECS_BUILD_BENCHES)
    find_package(benchmark REQUIRED)

    add_executable(necsbm
            benches/bench.cpp
    )

    target_include_directories(necsbm PRIVATE
            include
    )

    target_link_libraries(necsbm PRIVATE
            necs
            benchmark::benchmark
    )

    if(Gperftools_FOUND)
        target_link_libraries(necsbm PRIVATE ${GPERFTOOLS_PROFILER_LIBRARY})
        message(STATUS "Gperftools found, enabling profiling support")
    else()
        target_link_libraries(necsbm PRIVATE profiler)
        message(STATUS "Using direct profiler linking")
    endif()

    set(BENCHMARKS_FLAGS
            "-O3"
            "-march=native"
            "-fno-omit-frame-pointer"
    )

    target_compile_options(necsbm PRIVATE
            ${BENCHMARKS_FLAGS}
    )
endif ()