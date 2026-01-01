# ==============================================================================
# lgeneral - Main game executable
# ==============================================================================

# Minimum CMake version required for features used in this directory
cmake_minimum_required(VERSION 3.16)

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------
# Collect main source files
file(GLOB_RECURSE LGENERAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Include util sources so functions like locale_init() are available
file(GLOB LGENERAL_UTIL_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/util/*.c"
)

# Select platform-specific path implementation exclusively
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # On Linux, use paths-linux.c and exclude the generic paths.c
    list(REMOVE_ITEM LGENERAL_UTIL_SOURCES "${PROJECT_SOURCE_DIR}/util/paths.c")
else()
    # On non-Linux platforms, use the generic paths.c
    list(REMOVE_ITEM LGENERAL_UTIL_SOURCES "${PROJECT_SOURCE_DIR}/util/paths-linux.c")
endif()

list(APPEND LGENERAL_SOURCES ${LGENERAL_UTIL_SOURCES})

# ------------------------------------------------------------------------------
# Executable target
# ------------------------------------------------------------------------------
add_executable(lgeneral ${LGENERAL_SOURCES})

# Include directories
target_include_directories(lgeneral PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/util
)

# ------------------------------------------------------------------------------
# Dependencies and libraries
# ------------------------------------------------------------------------------
# Link configuration interface library
target_link_libraries(lgeneral PRIVATE
    lgeneral_config
    lgeneral::platform
)
if(LG_HAVE_SDL)
    target_link_libraries(lgeneral PRIVATE lgeneral::sdl)
endif()

if(LG_HAVE_SDL_MIXER)
    target_link_libraries(lgeneral PRIVATE lgeneral::sdl_mixer)
endif()

if(LG_HAVE_INTL)
    target_link_libraries(lgeneral PRIVATE lgeneral::intl)
endif()

# ------------------------------------------------------------------------------
# Output configuration
# ------------------------------------------------------------------------------
set_target_properties(lgeneral PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ------------------------------------------------------------------------------
# Runtime data deployment
# ------------------------------------------------------------------------------
# Copy runtime data directories to build directory for development builds
set(LGENERAL_DATA_DIRS
    themes
    scenarios
    campaigns
    nations
    units
    sounds
    gfx
    maps
    terrain
    music
    ai_modules
)

foreach(_dir IN LISTS LGENERAL_DATA_DIRS)
    if(EXISTS "${PROJECT_SOURCE_DIR}/src/${_dir}")
        add_custom_command(TARGET lgeneral POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/bin/${_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${PROJECT_SOURCE_DIR}/src/${_dir}"
                "${CMAKE_BINARY_DIR}/bin/${_dir}"
            COMMENT "Copying ${_dir} to build directory"
        )
    endif()
endforeach()

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------
install(TARGETS lgeneral
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install game data files if requested
if(INSTALL_DATA)
    foreach(_dir IN LISTS LGENERAL_DATA_DIRS)
        if(EXISTS "${PROJECT_SOURCE_DIR}/src/${_dir}")
            install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/${_dir}"
                DESTINATION ${CMAKE_INSTALL_DATADIR}/lgeneral
            )
        endif()
    endforeach()
endif()
