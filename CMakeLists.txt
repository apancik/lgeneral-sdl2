# ==============================================================================
# lgeneral - Turn-based strategy game
# ==============================================================================
# Minimum CMake version required
cmake_minimum_required(VERSION 3.16)

# Set policies for modern CMake behavior
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)  # find_package uses <PackageName>_ROOT variables
endif()

# Project declaration
project(lgeneral VERSION 1.4.4 LANGUAGES C)

# ==============================================================================
# Standard includes and configuration
# ==============================================================================
include(GNUInstallDirs)

# Place all executables in a single bin/ folder inside the build tree
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ==============================================================================
# Compiler configuration
# ==============================================================================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Silence warnings about taking the address of packed struct members on Clang
if(CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
	add_compile_options(-Wno-address-of-packed-member)
endif()

# Project-level variables used in the generated config.h
set(PACKAGE "lgeneral")
set(TOP_SRCDIR ".")

# Project-wide compile definitions
if(UNIX)
    add_compile_definitions(_GNU_SOURCE)
endif()

# Prefer explicit control of shared vs static; downstream can override.
option(BUILD_SHARED_LIBS "Build libraries as shared instead of static" OFF)

# On Windows, let CMake export symbols automatically for libraries built here.
if(WIN32)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# ==============================================================================
# Feature detection and platform checks
# ==============================================================================
# Replicate important Autoconf HAVE_* tests using CMake checks
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckLibraryExists)

find_package(PkgConfig QUIET)

check_include_files("string.h;strings.h" HAVE_STRING_H)
check_include_files("locale.h" HAVE_LOCALE_H)

check_symbol_exists(strcasestr "string.h" HAVE_STRCASESTR)

# Check for setenv/putenv (portability helpers)
check_function_exists(setenv HAVE_SETENV)
check_function_exists(putenv HAVE_PUTENV)

# Additional portability and stdlib helpers (port common Autoconf checks)
check_function_exists(snprintf HAVE_SNPRINTF)
check_function_exists(vsnprintf HAVE_VSNPRINTF)
check_function_exists(asprintf HAVE_ASPRINTF)
check_function_exists(vasprintf HAVE_VASPRINTF)
check_function_exists(newlocale HAVE_NEWLOCALE)
check_function_exists(wprintf HAVE_WPRINTF)

# Normalize checks: if a HAVE_* check was written into the cache as 0,
# unset it so `#cmakedefine01` produces an undefined macro instead of
# a `#define MACRO 0` which can cause redefinition warnings.
set(_HAVE_NORMALIZE
	HAVE_VASPRINTF
	HAVE_ASPRINTF
	HAVE_NEWLOCALE
	HAVE_WPRINTF
)
foreach(_h IN LISTS _HAVE_NORMALIZE)
	if(DEFINED ${_h} AND "${${_h}}" STREQUAL "0")
		unset(${_h})
		unset(${_h} CACHE)
	endif()
endforeach()

# ==============================================================================
# Configuration header generation
# ==============================================================================
# configure_file will process #cmakedefine and @PACKAGE@/@VERSION@
# Generate the single configured header used by the project. Sources include
# `lgeneral_config.h` (see `src/config.h`) so we only generate that name.
configure_file(${CMAKE_SOURCE_DIR}/cmake/config.h.in ${CMAKE_BINARY_DIR}/lgeneral_config.h)

add_library(lgeneral_config INTERFACE)
target_include_directories(lgeneral_config INTERFACE ${CMAKE_BINARY_DIR})
target_compile_definitions(lgeneral_config INTERFACE HAVE_CONFIG_H)
add_library(lgeneral::config ALIAS lgeneral_config)

# Minimal platform interface for common system libs
add_library(lgeneral_platform INTERFACE)
if(UNIX)
	target_link_libraries(lgeneral_platform INTERFACE m)
endif()
add_library(lgeneral::platform ALIAS lgeneral_platform)

# ==============================================================================
# Build options
# ==============================================================================
option(USE_SDL "Enable SDL support (if available)" ON)
option(BUILD_LGED "Build the GTK-based lged editor" OFF)
option(BUILD_RGEDIT "Build the GTK-based lgeneral-redit editor" OFF)

# Installation options
option(INSTALL_DATA "Install game data files" ON)

# ==============================================================================
# Shared dependency discovery
# ==============================================================================
set(LG_HAVE_SDL OFF)
set(LG_HAVE_SDL_MIXER OFF)
set(LG_HAVE_INTL OFF)

if(USE_SDL)
	find_package(SDL2 QUIET)

	if(TARGET SDL2::SDL2)
		add_library(lgeneral_sdl INTERFACE)
		target_link_libraries(lgeneral_sdl INTERFACE SDL2::SDL2)
		# Propagate includes from SDL2::SDL2 target
		get_target_property(_lg_sdl_inc SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
		if(_lg_sdl_inc)
			target_include_directories(lgeneral_sdl INTERFACE ${_lg_sdl_inc})
		endif()
		add_library(lgeneral::sdl ALIAS lgeneral_sdl)
		set(LG_HAVE_SDL ON)
		message(STATUS "  SDL2: Found (imported target)")
	elseif(SDL2_FOUND)
		add_library(lgeneral_sdl INTERFACE)
		target_include_directories(lgeneral_sdl INTERFACE ${SDL2_INCLUDE_DIRS})
		target_link_libraries(lgeneral_sdl INTERFACE ${SDL2_LIBRARIES})
		add_library(lgeneral::sdl ALIAS lgeneral_sdl)
		set(LG_HAVE_SDL ON)
		message(STATUS "  SDL2: Found (legacy variables)")
	else()
		message(WARNING "SDL2 not found; disabling SDL support")
		set(USE_SDL OFF CACHE BOOL "Enable SDL support (if available)" FORCE)
	endif()
endif()

if(LG_HAVE_SDL)
	find_package(SDL2_mixer CONFIG QUIET)
	if(NOT TARGET SDL2_mixer::SDL2_mixer)
		find_package(SDL2_mixer QUIET)
	endif()

	if(TARGET SDL2_mixer::SDL2_mixer)
		add_library(lgeneral_sdl_mixer INTERFACE)
		target_link_libraries(lgeneral_sdl_mixer INTERFACE SDL2_mixer::SDL2_mixer)
		target_compile_definitions(lgeneral_sdl_mixer INTERFACE WITH_SOUND)
		add_library(lgeneral::sdl_mixer ALIAS lgeneral_sdl_mixer)
		set(LG_HAVE_SDL_MIXER ON)
		message(STATUS "  SDL_mixer: Found (CMake package)")
	elseif(SDL2_MIXER_FOUND AND SDL2_MIXER_LIBRARIES)
		add_library(lgeneral_sdl_mixer INTERFACE)
		target_include_directories(lgeneral_sdl_mixer INTERFACE ${SDL2_MIXER_INCLUDE_DIRS})
		target_compile_definitions(lgeneral_sdl_mixer INTERFACE WITH_SOUND)
		target_link_libraries(lgeneral_sdl_mixer INTERFACE ${SDL2_MIXER_LIBRARIES})
		add_library(lgeneral::sdl_mixer ALIAS lgeneral_sdl_mixer)
		set(LG_HAVE_SDL_MIXER ON)
		message(STATUS "  SDL_mixer: Found (module variables)")
	elseif(PkgConfig_FOUND)
		pkg_check_modules(SDL_MIXER QUIET IMPORTED_TARGET SDL2_mixer)
		if(NOT TARGET PkgConfig::SDL_MIXER)
			pkg_check_modules(SDL_MIXER QUIET IMPORTED_TARGET SDL_mixer)
		endif()

		if(TARGET PkgConfig::SDL_MIXER)
			add_library(lgeneral_sdl_mixer INTERFACE)
			target_link_libraries(lgeneral_sdl_mixer INTERFACE PkgConfig::SDL_MIXER)
			target_compile_definitions(lgeneral_sdl_mixer INTERFACE WITH_SOUND)
			add_library(lgeneral::sdl_mixer ALIAS lgeneral_sdl_mixer)
			set(LG_HAVE_SDL_MIXER ON)
			message(STATUS "  SDL_mixer: Found (pkg-config)")
		elseif(DEFINED SDL_MIXER_LIBRARIES AND SDL_MIXER_LIBRARIES)
			add_library(lgeneral_sdl_mixer INTERFACE)
			target_include_directories(lgeneral_sdl_mixer INTERFACE ${SDL_MIXER_INCLUDE_DIRS})
			target_compile_definitions(lgeneral_sdl_mixer INTERFACE WITH_SOUND)
			target_link_libraries(lgeneral_sdl_mixer INTERFACE ${SDL_MIXER_LIBRARIES})
			add_library(lgeneral::sdl_mixer ALIAS lgeneral_sdl_mixer)
			set(LG_HAVE_SDL_MIXER ON)
			message(STATUS "  SDL_mixer: Found (pkg-config legacy variables)")
		endif()
	endif()
elseif(USE_SDL)
	message(STATUS "  SDL_mixer: Skipped (SDL disabled)")
endif()

if(PkgConfig_FOUND AND NOT TARGET lgeneral_intl)
	pkg_check_modules(GETTEXT QUIET IMPORTED_TARGET libintl)
	if(TARGET PkgConfig::GETTEXT)
		add_library(lgeneral_intl INTERFACE)
		target_link_libraries(lgeneral_intl INTERFACE PkgConfig::GETTEXT)
		add_library(lgeneral::intl ALIAS lgeneral_intl)
		set(LG_HAVE_INTL ON)
	endif()
endif()

if(NOT TARGET lgeneral_intl)
	find_library(INTL_LIB intl)
	if(INTL_LIB)
		add_library(lgeneral_intl INTERFACE)
		target_link_libraries(lgeneral_intl INTERFACE ${INTL_LIB})
		find_path(INTL_INCLUDE_DIR libintl.h
			HINTS
				/opt/homebrew/opt/gettext/include
				/usr/local/opt/gettext/include
				/usr/local/include
				/usr/include)
		if(INTL_INCLUDE_DIR)
			target_include_directories(lgeneral_intl INTERFACE ${INTL_INCLUDE_DIR})
		endif()
		add_library(lgeneral::intl ALIAS lgeneral_intl)
		set(LG_HAVE_INTL ON)
	endif()
endif()

# ==============================================================================
# Subdirectories
# ==============================================================================
add_subdirectory(src)
add_subdirectory(lgc-pg)
if(BUILD_LGED)
	add_subdirectory(lged)
else()
	message(STATUS "Skipping lged (GTK editor). Enable with -DBUILD_LGED=ON")
endif()

if(BUILD_RGEDIT)
	add_subdirectory(lgeneral-redit)
else()
	message(STATUS "Skipping lgeneral-redit (GTK editor). Enable with -DBUILD_RGEDIT=ON")
endif()

# ==============================================================================
# Configuration summary
# ==============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "lgeneral ${PROJECT_VERSION} - Configuration Summary")
message(STATUS "========================================")
message(STATUS "  SDL requested:     ${USE_SDL}")
message(STATUS "  SDL available:     ${LG_HAVE_SDL}")
message(STATUS "  SDL_mixer audio:   ${LG_HAVE_SDL_MIXER}")
message(STATUS "  libintl:           ${LG_HAVE_INTL}")
message(STATUS "  Build lged:        ${BUILD_LGED}")
message(STATUS "  Build redit:       ${BUILD_RGEDIT}")
message(STATUS "  Install data:      ${INSTALL_DATA}")
message(STATUS "  Install prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  C compiler:        ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "========================================")
message(STATUS "")
