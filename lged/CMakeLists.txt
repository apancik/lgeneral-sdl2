file(GLOB_RECURSE LGED_SRCS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(lged ${LGED_SRCS})

target_include_directories(lged PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/util
)

if(EXISTS ${PROJECT_SOURCE_DIR}/config.h)
    target_compile_definitions(lged PRIVATE HAVE_CONFIG_H)
endif()

target_link_libraries(lged PRIVATE m)
set_target_properties(lged PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
