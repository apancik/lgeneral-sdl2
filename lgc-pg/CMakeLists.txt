# ==============================================================================
# lgc-pg - Panzer General campaign converter
# ==============================================================================

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------
# Collect all source files
file(GLOB_RECURSE LGC_PG_ALL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Exclude shptool.c (separate utility)
set(LGC_PG_SOURCES ${LGC_PG_ALL_SOURCES})
list(REMOVE_ITEM LGC_PG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/shptool.c")

# ------------------------------------------------------------------------------
# Executable target
# ------------------------------------------------------------------------------
add_executable(lgc-pg ${LGC_PG_SOURCES})

# ------------------------------------------------------------------------------
# Include directories
# ------------------------------------------------------------------------------
target_include_directories(lgc-pg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/util
)

# ------------------------------------------------------------------------------
# Dependencies and libraries
# ------------------------------------------------------------------------------
# Link configuration interface library
target_link_libraries(lgc-pg PRIVATE
    lgeneral_config
    lgeneral::platform
)

if(LG_HAVE_SDL)
    target_link_libraries(lgc-pg PRIVATE lgeneral::sdl)
endif()

# ------------------------------------------------------------------------------
# Output configuration
# ------------------------------------------------------------------------------
set_target_properties(lgc-pg PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ------------------------------------------------------------------------------
# Runtime data deployment
# ------------------------------------------------------------------------------
# Copy runtime convdata resources to build directory
add_custom_command(TARGET lgc-pg POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/convdata"
        "${CMAKE_BINARY_DIR}/bin/convdata"
    COMMENT "Copying convdata to build directory"
)

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------
install(TARGETS lgc-pg
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install convdata resources
if(INSTALL_DATA)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/convdata"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lgc-pg
    )
endif()

# ==============================================================================
# shptool - Shape file utility
# ==============================================================================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shptool.c")
    # ------------------------------------------------------------------------------
    # Source files
    # ------------------------------------------------------------------------------
    # Minimal source set (avoid pulling in globals from main.c)
    set(SHPTOOL_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/shptool.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/shp.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/misc.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/lgcpg_globals.c"
    )
    
    # ------------------------------------------------------------------------------
    # Executable target
    # ------------------------------------------------------------------------------
    add_executable(shptool ${SHPTOOL_SOURCES})
    
    # ------------------------------------------------------------------------------
    # Include directories
    # ------------------------------------------------------------------------------
    target_include_directories(shptool PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/util
    )
    
    # ------------------------------------------------------------------------------
    # Dependencies and libraries
    # ------------------------------------------------------------------------------
    # Link configuration interface library
    target_link_libraries(shptool PRIVATE
        lgeneral_config
        lgeneral::platform
    )
    
    if(LG_HAVE_SDL)
        target_link_libraries(shptool PRIVATE lgeneral::sdl)
    endif()
    
    # ------------------------------------------------------------------------------
    # Output configuration
    # ------------------------------------------------------------------------------
    set_target_properties(shptool PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS YES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    
    # ------------------------------------------------------------------------------
    # Installation
    # ------------------------------------------------------------------------------
    install(TARGETS shptool
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
