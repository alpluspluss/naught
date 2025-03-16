# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

cmake_minimum_required(VERSION 3.29)
project(naught LANGUAGES C CXX)

# render lib
option(FORGE_BUILD_TESTS "Forge Build Tests" ON)
option(FORGE_BUILD_BENCHES "Forge Build Benches" ON)

# ECS
option(NCS_BUILD_TESTS "Naught ECS Build Tests" ON)
option(NCS_BUILD_BENCHES "Naught Build Benches " ON)

# engine
option(NAUGHT_BUILD_TESTS "Naught Build Tests" OFF)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(forge)
add_subdirectory(naught)
add_subdirectory(ncs)

# test status message
message(STATUS "Forge Build Tests: ${FORGE_BUILD_TESTS}")
message(STATUS "Naught ECS Build Tests: ${NCS_BUILD_TESTS}")
message(STATUS "Naught Build Tests: ${NAUGHT_BUILD_TESTS}")

# bench status message
message(STATUS "Forge Build Benches: ${FORGE_BUILD_BENCHES}")
message(STATUS "Naught ECS Build Benches: ${NCS_BUILD_BENCHES}")