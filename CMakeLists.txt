# CMAKE project for openrct2
cmake_minimum_required(VERSION 3.5)
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

project(openrct2 C CXX)

include(FindPkgConfig)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

set(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(CMAKE_MACOSX_RPATH 1)

set(TITLE_SEQUENCE_URL  "https://github.com/OpenRCT2/title-sequences/releases/download/v0.1.2/title-sequence-v0.1.2.zip")
set(TITLE_SEQUENCE_SHA1 "1136ef92bfb05cd1cba9831ba6dc4a653d87a246")

option(FORCE32 "Force 32-bit build. It will add `-m32` to compiler flags.")
option(WITH_TESTS "Build tests")
option(PORTABLE "Create a portable build (-rpath=$ORIGIN)" OFF)
option(DOWNLOAD_TITLE_SEQUENCES "Download title sequences during installation." ON)

# Options
option(STATIC "Create a static build.")
option(USE_MMAP "Use mmap to try loading rct2's data segment into memory.")

option(DISABLE_HTTP_TWITCH "Disable HTTP and Twitch support.")
option(DISABLE_NETWORK "Disable multiplayer functionality. Mainly for testing.")
option(DISABLE_TTF "Disable support for TTF provided by freetype2.")
option(ENABLE_LIGHTFX "Enable lighting effects." ON)

if (FORCE32)
    set(TARGET_M "-m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TARGET_M}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${TARGET_M}")
endif ()

if (PORTABLE OR WIN32)
    set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_BINDIR}")
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif ()

# Define current git branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${ROOT_DIR}
    OUTPUT_VARIABLE OPENRCT2_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Define short commit hash
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${ROOT_DIR}
    OUTPUT_VARIABLE OPENRCT2_COMMIT_SHA1_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

function (ADD_CHECK_CXX_COMPILER_FLAG _CXXFLAGS _CACHE_VAR _FLAG)
    CHECK_CXX_COMPILER_FLAG("${_FLAG}" "${_CACHE_VAR}")
    if (${_CACHE_VAR})
        set(${_CXXFLAGS} "${${_CXXFLAGS}} ${_FLAG}" PARENT_SCOPE)
    else ()
        message(STATUS "Unsupported CXXFLAG: ${_FLAG}")
    endif ()
endfunction ()

ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_NULL_DEREFERENCE -Wnull-dereference)
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_SUGGEST_FINAL_TYPES -Wsuggest-final-types)
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_SUGGEST_FINAL_METHODS -Wsuggest-final-methods)
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_SUGGEST_OVERRIDE -Wsuggest-override)
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_DUPLICATED_COND -Wduplicated-cond)
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_NON_VIRTUAL_DTOR -Wnon-virtual-dtor)
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_MISSING_VARIABLE_DECLARATIONS -Wmissing-variable-declarations)
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_DUPLICATED_BRANCHES -Wduplicated-branches)
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_RESTRICT -Wrestrict)

if (MINGW)
    ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_SUGGEST_OVERRIDE -Wno-error=redundant-decls)
endif ()

# Items below are not supported by ICC
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_REDUNDANT_DECLS -Wredundant-decls)
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_IGNORED_QUALIFIERS -Wignored-qualifiers)

# -Wstrict-overflow is only active when -fstrict-overflow is enabled, but -fstrict-overflow
# is enabled on -O2, -O3, -Os. This should help catch bugs locally before they reach Travis
# As of 2a435bf -Wstrict-overflow=1 passes, but higher values do not.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstrict-overflow")
ADD_CHECK_CXX_COMPILER_FLAG(CMAKE_CXX_FLAGS CXX_WARN_STRICT_OVERFLOW -Wstrict-overflow=1)

# Compiler flags
set(DEBUG_LEVEL 0 CACHE STRING "Select debug level for compilation. Use value in range 0–3.")
if (NINTENDO_3DS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -fstrict-aliasing -Wno-error -Wundef -Winit-self -Wall -Wno-unknown-pragmas -Wno-unused-function -Wno-missing-braces -Wno-narrowing -Wno-deprecated-declarations ")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -fstrict-aliasing -Wno-error ")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -fstrict-aliasing -Werror -Wno-error=null-dereference -Wundef -Wmissing-declarations -Winit-self -Wall -Wno-unknown-pragmas -Wno-unused-function -Wno-missing-braces ")
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comment -Wshadow -Wnonnull")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG=${DEBUG_LEVEL}")

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=objc-method-access")
endif()

# On mingw all code is already PIC, this will avoid compiler error on redefining this option
if (NOT MINGW AND NOT VITA AND NOT NINTENDO_3DS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif ()

if (APPLE AND NOT USE_MMAP)
    set(PIE_FLAG "-fno-pie")
else ()
    set(PIE_FLAG "-fpie")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")

# Defines
if (USE_MMAP)
    add_definitions(-DUSE_MMAP)
endif ()
if (DISABLE_NETWORK)
    add_definitions(-DDISABLE_NETWORK)
endif ()
if (DISABLE_HTTP_TWITCH)
    add_definitions(-DDISABLE_HTTP)
    add_definitions(-DDISABLE_TWITCH)
endif ()
if (DISABLE_TTF)
    add_definitions(-DNO_TTF)
endif ()
if (ENABLE_LIGHTFX)
    add_definitions(-D__ENABLE_LIGHTFX__)
endif ()

if (CXX_WARN_SUGGEST_FINAL_TYPES)
    # Disable -Wsuggest-final-types via pragmas where due.
   add_definitions(-D__WARN_SUGGEST_FINAL_TYPES__)
endif ()

if (CXX_WARN_SUGGEST_FINAL_METHODS)
    # Disable -Wsuggest-final-methods via pragmas where due.
   add_definitions(-D__WARN_SUGGEST_FINAL_METHODS__)
endif ()

if(EXISTS "${ROOT_DIR}/discord-rpc")
    # Don't build discord's examples, some of which are in C and do not honour
    # the flags we set for C++. Also we don't use the provided examples.
    set(BUILD_EXAMPLES OFF CACHE BOOL "Build example apps")
    add_subdirectory("${ROOT_DIR}/discord-rpc")
    add_definitions(-D__ENABLE_DISCORD__)
    include_directories("${ROOT_DIR}/discord-rpc/include")
    set(HAVE_DISCORD_RPC TRUE)
    message("Building with discord-rpc support")
else()
	message("No discord-rpc detected, to enable clone discord-rpc to root directory: ${ROOT_DIR}")
endif()

# Copied from https://github.com/opencv/opencv/blob/dcdd6af5a856826fe62c95322145731e702e54c5/cmake/OpenCVDetectCXXCompiler.cmake#L63-L70
if(MSVC64 OR MINGW64)
    set(X86_64 1)
elseif(MINGW OR (MSVC AND NOT CMAKE_CROSSCOMPILING))
    set(X86 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
    set(X86_64 1)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|i386.*|x86.*|amd64.*|AMD64.*")
    set(X86 1)
endif()

# Include sub-projects
include("${ROOT_DIR}/src/openrct2/CMakeLists.txt" NO_POLICY_SCOPE)
if (NOT NINTENDO_3DS)
    include("${ROOT_DIR}/src/openrct2-cli/CMakeLists.txt" NO_POLICY_SCOPE)
endif ()
include("${ROOT_DIR}/src/openrct2-ui/CMakeLists.txt" NO_POLICY_SCOPE)

