# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

cmake_minimum_required(VERSION 3.29)
project(naught LANGUAGES C CXX)

# ECS
option(NCS_BUILD_TESTS "Naught ECS Build Tests" ON)
option(NCS_BUILD_BENCHES "Naught Build Benches " ON)

# engine
option(NAUGHT_BUILD_TESTS "Naught Build Tests" OFF)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(naught)
add_subdirectory(ncs)

# test status message
message("-- Naught ECS Build Tests: ${NECS_BUILD_TESTS}")
message("-- Naught Build Tests: ${NAUGHT_BUILD_TESTS}")

# bench status message
message("-- Naught ECS Build Benches: ${NECS_BUILD_BENCHES}")