
file(GLOB_RECURSE LGCPG_ALL_SRCS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Sources for the main lgc-pg tool (exclude shptool.c which is a separate utility)
set(LGCPG_SRCS ${LGCPG_ALL_SRCS})
list(REMOVE_ITEM LGCPG_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/shptool.c")

add_executable(lgc-pg ${LGCPG_SRCS})

target_include_directories(lgc-pg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/util
)

if(EXISTS ${PROJECT_SOURCE_DIR}/config.h)
    target_compile_definitions(lgc-pg PRIVATE HAVE_CONFIG_H)
endif()

if(USE_SDL)
    # Prefer SDL 1.x via sdl-config if present
    find_program(SDL_CONFIG sdl-config)
    if(SDL_CONFIG)
        execute_process(COMMAND ${SDL_CONFIG} --cflags
            OUTPUT_VARIABLE LGCPG_SDL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${SDL_CONFIG} --libs
            OUTPUT_VARIABLE LGCPG_SDL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(LGCPG_SDL_CFLAGS)
            separate_arguments(LGCPG_SDL_CFLAG_LIST UNIX_COMMAND "${LGCPG_SDL_CFLAGS}")
            target_compile_options(lgc-pg PRIVATE ${LGCPG_SDL_CFLAG_LIST})
        endif()
        if(LGCPG_SDL_LIBS)
            separate_arguments(LGCPG_SDL_LIB_LIST UNIX_COMMAND "${LGCPG_SDL_LIBS}")
            target_link_libraries(lgc-pg PRIVATE ${LGCPG_SDL_LIB_LIST})
        endif()
    else()
        find_package(SDL2 QUIET)
        if(SDL2_FOUND)
            target_include_directories(lgc-pg PRIVATE ${SDL2_INCLUDE_DIRS})
            target_link_libraries(lgc-pg PRIVATE ${SDL2_LIBRARIES})
        endif()
    endif()
endif()

target_link_libraries(lgc-pg PRIVATE m)
set_target_properties(lgc-pg PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Ensure the runtime `convdata` resources are available next to the built
# executable. This copies the `convdata` source directory into the target
# runtime output directory after building `lgc-pg` so running from the
# build tree (or from `${CMAKE_BINARY_DIR}/bin`) finds `./convdata`.
add_custom_command(TARGET lgc-pg POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/convdata
        ${CMAKE_BINARY_DIR}/bin/convdata
    COMMENT "Copying lgc-pg/convdata -> ${CMAKE_BINARY_DIR}/bin/convdata"
)

# Build the `shptool` utility as a separate executable
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shptool.c")
    # Build shptool from a minimal set of sources (avoid pulling in globals
    # from main.c). Only include shptool, shp and misc implementations.
    set(SHPT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/shptool.c
        ${CMAKE_CURRENT_SOURCE_DIR}/shp.c
        ${CMAKE_CURRENT_SOURCE_DIR}/misc.c
    )
    add_executable(shptool ${SHPT_SRCS})
    target_include_directories(shptool PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/util
    )
    if(EXISTS ${PROJECT_SOURCE_DIR}/config.h)
        target_compile_definitions(shptool PRIVATE HAVE_CONFIG_H)
    endif()
    if(USE_SDL AND LGCPG_SDL_CFLAG_LIST)
        target_compile_options(shptool PRIVATE ${LGCPG_SDL_CFLAG_LIST})
    endif()
    if(USE_SDL AND LGCPG_SDL_LIB_LIST)
        target_link_libraries(shptool PRIVATE ${LGCPG_SDL_LIB_LIST})
    endif()
    target_link_libraries(shptool PRIVATE m)
    set_target_properties(shptool PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
