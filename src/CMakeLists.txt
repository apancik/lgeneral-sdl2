# ==============================================================================
# lgeneral - Main game executable
# ==============================================================================

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

# Allow overriding the install data dir at configure time. If not provided,
# fall back to the computed install datadir under the install prefix.
if(NOT DEFINED INSTALLDIR)
    set(INSTALLDIR "${CMAKE_INSTALL_FULL_DATADIR}/lgeneral" CACHE PATH "Runtime data directory used by the binary (compile-time)")
else()
    # Accept user-specified value as a cache entry so it shows up in GUIs
    set(INSTALLDIR "${INSTALLDIR}" CACHE PATH "Runtime data directory used by the binary (compile-time)" FORCE)
endif()

target_compile_definitions(lgeneral PRIVATE
    "INSTALLDIR=\"${INSTALLDIR}\""
)

# ------------------------------------------------------------------------------
# Dependencies and libraries
# ------------------------------------------------------------------------------
# Link configuration interface library
target_link_libraries(lgeneral PRIVATE
    lgeneral::config
    lgeneral::platform
)
if(LG_HAVE_SDL)
    target_link_libraries(lgeneral PRIVATE lgeneral::sdl)
endif()

if(LG_HAVE_SDL_MIXER)
    target_link_libraries(lgeneral PRIVATE lgeneral::sdl_mixer)
endif()

if(LG_ENABLE_NLS)
    if(TARGET lgeneral::intl)
        target_link_libraries(lgeneral PRIVATE lgeneral::intl)
    endif()

    if(INTL_LIB)
        target_link_libraries(lgeneral PRIVATE ${INTL_LIB})
    endif()

    if(INTL_INCLUDE_DIR)
        target_include_directories(lgeneral PRIVATE ${INTL_INCLUDE_DIR})
    endif()
endif()

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

# Install man page
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/lgeneral.6"
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man6"
)

# Install desktop file (Linux only)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(FILES "${PROJECT_SOURCE_DIR}/lgeneral.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
    )
endif()

# Install game data files if requested
if(INSTALL_DATA)
    foreach(_dir IN LISTS LGENERAL_DATA_DIRS)
        if(EXISTS "${PROJECT_SOURCE_DIR}/src/${_dir}")
            install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/${_dir}"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/lgeneral"
            )
        endif()
    endforeach()
endif()
