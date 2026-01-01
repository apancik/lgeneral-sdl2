# ==============================================================================
# lgeneral-redit - GTK-based resource editor
# ==============================================================================

# Minimum CMake version required for features used in this directory
cmake_minimum_required(VERSION 3.16)

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------
# Collect source files
file(GLOB_RECURSE LGENERAL_REDIT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Exclude local copies to use project implementations
list(REMOVE_ITEM LGENERAL_REDIT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/parser.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/parser.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/list.c"
)

# ------------------------------------------------------------------------------
# Executable target
# ------------------------------------------------------------------------------
add_executable(lgeneral-redit ${LGENERAL_REDIT_SOURCES})

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------
target_include_directories(lgeneral-redit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/util
)

# ------------------------------------------------------------------------------
# Dependencies and libraries
# ------------------------------------------------------------------------------
# Link configuration interface library
target_link_libraries(lgeneral-redit PRIVATE lgeneral_config)

# GTK2 dependency
if(NOT PkgConfig_FOUND)
    message(FATAL_ERROR "PkgConfig required for GTK2 detection. Install pkg-config or configure with -DBUILD_RGEDIT=OFF")
endif()

pkg_check_modules(GTK2 REQUIRED IMPORTED_TARGET gtk+-2.0)
if(NOT TARGET PkgConfig::GTK2)
    message(FATAL_ERROR "GTK2 (gtk+-2.0) not found. Install GTK2 development packages or configure with -DBUILD_RGEDIT=OFF")
endif()

# Link libraries
target_link_libraries(lgeneral-redit PRIVATE
    lgeneral::platform
    PkgConfig::GTK2
)

# ------------------------------------------------------------------------------
# Additional source files
# ------------------------------------------------------------------------------
# Add shared project source files
target_sources(lgeneral-redit PRIVATE
    "${PROJECT_SOURCE_DIR}/src/parser.c"
    "${PROJECT_SOURCE_DIR}/src/list.c"
)

# Add platform-specific path implementation (same as main app)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(lgeneral-redit PRIVATE "${PROJECT_SOURCE_DIR}/util/paths-linux.c")
else()
    target_sources(lgeneral-redit PRIVATE "${PROJECT_SOURCE_DIR}/util/paths.c")
endif()

# ------------------------------------------------------------------------------
# Compile definitions
# ------------------------------------------------------------------------------
target_compile_definitions(lgeneral-redit PRIVATE
    TOP_SRCDIR="."
    PACKAGE_DATA_DIR="${PROJECT_SOURCE_DIR}"
    PACKAGE_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

# ------------------------------------------------------------------------------
# GTK compatibility
# ------------------------------------------------------------------------------
# Add compatibility helper for GTK2
target_sources(lgeneral-redit PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/compat.c"
)

# Force-include compat header for GTK helper prototypes (compiler-specific: GCC/Clang vs MSVC)
target_compile_options(lgeneral-redit PRIVATE
    "$<$<NOT:$<C_COMPILER_ID:MSVC>>:-include${CMAKE_CURRENT_SOURCE_DIR}/src/compat.h>"
    "$<$<C_COMPILER_ID:MSVC>:/FI${CMAKE_CURRENT_SOURCE_DIR}/src/compat.h>"
)

# ------------------------------------------------------------------------------
# Output configuration
# ------------------------------------------------------------------------------
set_target_properties(lgeneral-redit PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------
install(TARGETS lgeneral-redit
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
