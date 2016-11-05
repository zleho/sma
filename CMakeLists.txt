cmake_minimum_required(VERSION 3.5)

find_package(PkgConfig)
set (CMAKE_CXX_STANDARD 14)

pkg_search_module(PULSE REQUIRED libpulse-mainloop-glib)
pkg_search_module(GTKMM REQUIRED gtkmm-3.0)

link_directories(
    ${PULSE_LIBRARY_DIRS}
    ${GTKMM_LIBRARY_DIRS}
    )

include_directories(
    ${PULSE_INCLUDE_DIRS}
    ${GTKMM_INCLUDE_DIRS}
    )

add_executable(sma 
    src/main.cpp
    src/Config.cpp
    src/Application.cpp
    )

target_compile_options(sma PUBLIC 
    -Wall 
    -pedantic 
    -Werror 
    -pedantic-errors
    )

target_link_libraries(sma 
    ${PULSE_LIBRARIES}
    ${PULSE_LDFLAGS_OTHER}
    ${GTKMM_LIBRARIES}
    ${GTKMM_LDFLAGS_OTHER}
    )

target_compile_options(sma PUBLIC ${PULSE_CFLAGS_OTHER} ${GTKMM_CFLAGS_OTHER})
