# this file is a part of Naught Engine which is under MIT license; see LICENSE for more info

# naught lib

set(NAUGHT_SOURCE)

if (APPLE)
    list(APPEND NAUGHT_SOURCE
            lib/os/macos/window.mm
            lib/os/macos/app.mm
    )
endif ()

add_library(naught
        ${NAUGHT_SOURCE}
)

target_include_directories(naught PRIVATE
        ${PROJECT_SOURCE_DIR}/naught/include
        ${PROJECT_SOURCE_DIR}/necs/include
)

if (APPLE)
        target_link_libraries(naught PRIVATE
                "-framework AppKit"
                "-framework Cocoa"
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