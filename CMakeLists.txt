cmake_minimum_required(VERSION 3.10)
project(lgeneral C)

add_compile_options(-std=gnu89 -fgnu89-inline)

option(USE_SDL "Enable SDL support (if available)" ON)
option(BUILD_LGED "Build the GTK-based lged editor" OFF)

add_subdirectory(src)
add_subdirectory(lgc-pg)
if(BUILD_LGED)
	add_subdirectory(lged)
else()
	message(STATUS "Skipping lged (GTK editor). Enable with -DBUILD_LGED=ON")
endif()

message(STATUS "Top-level CMake for lgeneral configured. Use -DUSE_SDL=ON/OFF and -DBUILD_LGED=ON/OFF to configure builds.")
