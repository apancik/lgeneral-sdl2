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
    lgeneral::config
    lgeneral::platform
)

# Provide install-time data root so converter can find convdata after install
if(NOT DEFINED INSTALLDIR)
    set(INSTALLDIR "${CMAKE_INSTALL_FULL_DATADIR}/lgc-pg" CACHE PATH "Runtime data directory used by the binary (compile-time)")
else()
    set(INSTALLDIR "${INSTALLDIR}" CACHE PATH "Runtime data directory used by the binary (compile-time)" FORCE)
endif()

target_compile_definitions(lgc-pg PRIVATE
    "INSTALLDIR=\"${INSTALLDIR}\""
)

if(LG_HAVE_SDL)
    target_link_libraries(lgc-pg PRIVATE lgeneral::sdl)
endif()

# ------------------------------------------------------------------------------
# Runtime data deployment
# ------------------------------------------------------------------------------
# Copy runtime convdata resources to build directory if present
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/convdata")
    add_custom_command(TARGET lgc-pg POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/convdata"
            "${CMAKE_BINARY_DIR}/bin/convdata"
        COMMENT "Copying convdata to build directory"
    )
endif()

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------
install(TARGETS lgc-pg
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install man page
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/lgc-pg.1"
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
)

# Install convdata resources
if(INSTALL_DATA)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/convdata"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/lgc-pg"
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
            lgeneral::config
            lgeneral::platform
        )
    
    if(LG_HAVE_SDL)
        target_link_libraries(shptool PRIVATE lgeneral::sdl)
    endif()

    # Keep the helper tool aligned with the converter's data root
    if(NOT DEFINED INSTALLDIR)
        set(INSTALLDIR "${CMAKE_INSTALL_FULL_DATADIR}/lgc-pg" CACHE PATH "Runtime data directory used by the binary (compile-time)")
    else()
        set(INSTALLDIR "${INSTALLDIR}" CACHE PATH "Runtime data directory used by the binary (compile-time)" FORCE)
    endif()

    target_compile_definitions(shptool PRIVATE
        "INSTALLDIR=\"${INSTALLDIR}\""
    )
    
    # ------------------------------------------------------------------------------
    # Output configuration
    # ------------------------------------------------------------------------------
    
    # ------------------------------------------------------------------------------
    # Installation
    # ------------------------------------------------------------------------------
    install(TARGETS shptool
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
