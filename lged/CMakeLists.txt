# ==============================================================================
# lged - GTK-based level editor
# ==============================================================================

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------
# Collect source files
file(GLOB_RECURSE LGED_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Exclude local copies to use project implementations
list(REMOVE_ITEM LGED_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/parser.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/list.c"
)

# ------------------------------------------------------------------------------
# Executable target
# ------------------------------------------------------------------------------
add_executable(lged ${LGED_SOURCES})

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------
target_include_directories(lged PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/lgeneral-redit/src
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/util
)

# ------------------------------------------------------------------------------
# Compile definitions
# ------------------------------------------------------------------------------
target_compile_definitions(lged PRIVATE
    CMDLINE_ONLY
    TOP_SRCDIR="."
)

# ------------------------------------------------------------------------------
# Dependencies and libraries
# ------------------------------------------------------------------------------
# GTK2 dependency
if(NOT PkgConfig_FOUND)
    message(FATAL_ERROR "PkgConfig required for GTK2 detection. Install pkg-config or configure with -DBUILD_LGED=OFF")
endif()

pkg_check_modules(GTK2 REQUIRED IMPORTED_TARGET gtk+-2.0)
if(NOT TARGET PkgConfig::GTK2)
    message(FATAL_ERROR "GTK2 (gtk+-2.0) not found. Install GTK2 development packages or configure with -DBUILD_LGED=OFF")
endif()

# Link libraries
target_link_libraries(lged PRIVATE
    lgeneral_config
    lgeneral::platform
    PkgConfig::GTK2
)

# ------------------------------------------------------------------------------
# Additional source files
# ------------------------------------------------------------------------------
# Add shared project source files
target_sources(lged PRIVATE
    "${PROJECT_SOURCE_DIR}/src/parser.c"
    "${PROJECT_SOURCE_DIR}/src/list.c"
    "${PROJECT_SOURCE_DIR}/util/paths-linux.c"
)

# ------------------------------------------------------------------------------
# Output configuration
# ------------------------------------------------------------------------------
set_target_properties(lged PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------
install(TARGETS lged
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
