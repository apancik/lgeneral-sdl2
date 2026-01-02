# ==============================================================================
# lgeditor - SDL-based map editor
# ==============================================================================

# Runtime paths
# Keep install-time defaults but also record build-tree fallbacks for dev builds.
set(_LGEDITOR_RUNTIME_BIN "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
if(NOT _LGEDITOR_RUNTIME_BIN)
    set(_LGEDITOR_RUNTIME_BIN "${CMAKE_BINARY_DIR}/bin")
endif()

set(LGEDITOR_DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/lgeditor" CACHE PATH "Install-time runtime data directory for lgeditor assets")
set(LGEDITOR_LGENERAL_DIR "${CMAKE_INSTALL_FULL_BINDIR}" CACHE PATH "Install-time path to the LGeneral binary directory used by lgeditor")
set(LGEDITOR_DATADIR_BUILD "${_LGEDITOR_RUNTIME_BIN}/lgeditor-data")
set(LGEDITOR_LGENERAL_DIR_BUILD "${_LGEDITOR_RUNTIME_BIN}")

# ------------------------------------------------------------------------------
# Dependency checks
# ------------------------------------------------------------------------------
if(NOT LG_HAVE_SDL)
    message(FATAL_ERROR "SDL2 is required for lgeditor; configure with -DUSE_SDL=ON and ensure SDL2 is available.")
endif()

if(NOT TARGET lgeneral::sdl_image)
    message(FATAL_ERROR "SDL2_image is required for lgeditor; install SDL2_image development files or disable lgeditor.")
endif()

if(NOT TARGET lgeneral::sdl_ttf)
    message(FATAL_ERROR "SDL2_ttf is required for lgeditor; install SDL2_ttf development files or disable lgeditor.")
endif()

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
file(GLOB LGEDITOR_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB LGEDITOR_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# ------------------------------------------------------------------------------
# Executable
# ------------------------------------------------------------------------------
add_executable(lgeditor ${LGEDITOR_SOURCES} ${LGEDITOR_HEADERS})

# Includes
# Keep includes local; the editor is self-contained aside from SDL.
target_include_directories(lgeditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src    
)

# Linking
# Reuse shared configuration + platform helpers and SDL family dependencies.
target_link_libraries(lgeditor PRIVATE
    lgeneral::config
    lgeneral::platform
    lgeneral::sdl
    lgeneral::sdl_image
    lgeneral::sdl_ttf
)

# Compile definitions (string constants expected by the sources)
## Generate a small config header with runtime hints for both install and build trees.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/lgeditor_config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/lgeditor_config.h @ONLY)

target_include_directories(lgeditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Development convenience: copy data into the build tree.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data")
    add_custom_command(TARGET lgeditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${LGEDITOR_DATADIR_BUILD}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/data"
            "${LGEDITOR_DATADIR_BUILD}"
        COMMENT "Copying lgeditor data to build directory"
    )
endif()

# Installation
install(TARGETS lgeditor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lgeditor
    )
endif()
