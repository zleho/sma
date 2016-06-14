find_package(PkgConfig)

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

add_executable(sma main.cpp)

target_link_libraries(sma 
    ${PULSE_LIBRARIES}
    ${PULSE_LDFLAGS_OTHER}
    ${GTKMM_LIBRARIES}
    ${GTKMM_LDFLAGS_OTHER}
    )

target_compile_options(sma PUBLIC ${PULSE_CFLAGS_OTHER} ${GTKMM_CFLAGS_OTHER})

