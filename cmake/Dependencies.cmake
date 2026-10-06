set(WK_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SC_FLASH_BUILD_TOOLS OFF CACHE BOOL "" FORCE)

if(APPLE)
    set(ASTCENC_UNIVERSAL_BUILD OFF CACHE BOOL "" FORCE)
endif()

set(ASTCENC_WERROR OFF CACHE BOOL "" FORCE)

option(SC2FLA_FORCE_CSTDINT "Force-include <cstdint> in C++ code (needed with libstdc++ 15+; harmless elsewhere)" ON)
if(SC2FLA_FORCE_CSTDINT AND NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    string(APPEND CMAKE_CXX_FLAGS " -include cstdint")
endif()

if(APPLE)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        FetchContent_Declare(
            supercell-compression
            GIT_REPOSITORY https://github.com/sc-workshop/SupercellCompression.git
            GIT_TAG main
            PATCH_COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PatchAstcApple.py" supercell-compression/cmake/astc.cmake
        )
        FetchContent_MakeAvailable(supercell-compression)
    else()
        message(WARNING "python3 not found - cannot patch supercell-compression's cmake/astc.cmake for a non-Xcode Apple build; configure may fail with 'No target astcenc-sse4.1-static'")
    endif()
endif()

FetchContent_Declare(
    supercell-flash
    GIT_REPOSITORY https://github.com/fe4r-s/SupercellFlash.git
    GIT_TAG main
)
FetchContent_MakeAvailable(supercell-flash)

if(MINGW)
    set(_lzham_platform "${CMAKE_BINARY_DIR}/_deps/lzham_codec-src/lzhamdecomp/lzham_platform.cpp")
    if(EXISTS "${_lzham_platform}")
        file(READ "${_lzham_platform}" _lzham_text)
        string(REGEX REPLACE "#ifndef _MSC_VER([ \t]*[\r\n]+[ \t]*int sprintf_s)"
               "#if !defined(_MSC_VER) && !defined(__MINGW32__)\\1" _lzham_patched "${_lzham_text}")
        if(NOT _lzham_patched STREQUAL _lzham_text)
            file(WRITE "${_lzham_platform}" "${_lzham_patched}")
            message(STATUS "Patched lzham_platform.cpp for MinGW (sprintf_s/vsprintf_s already provided by the headers)")
        endif()
    else()
        message(WARNING "lzham_platform.cpp not found at ${_lzham_platform} - the MinGW sprintf_s patch was not applied")
    endif()

    if(TARGET lzham-lib)
        target_sources(lzham-lib PRIVATE
            "${CMAKE_BINARY_DIR}/_deps/lzham_codec-src/lzhamcomp/lzham_win32_threading.cpp")
    else()
        message(WARNING "target lzham-lib not found - the MinGW lzham threading fix was not applied")
    endif()
endif()

set(_wkcore_src_dir "${CMAKE_BINARY_DIR}/_deps/workshopcore-src")
set(_wkcore_pool_cpp "${_wkcore_src_dir}/workshop-core/source/core/3rdparty/Allocator/STL/MemoryPool.cpp")
if(EXISTS "${_wkcore_pool_cpp}")
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "
import re, sys
path = sys.argv[1]
src = open(path, encoding='utf-8').read()
if 'curr->prev->next = curr->next' in src:
    sys.exit(0)
old = 'm_alloc_block = curr->next;'
needle_count = src.count(old)
if needle_count != 1:
    sys.exit(1)
fixed = (
    'if (curr->prev != nullptr) {\\n'
    '\\t\\t\\t\\tcurr->prev->next = curr->next;\\n'
    '\\t\\t\\t} else {\\n'
    '\\t\\t\\t\\tm_alloc_block = curr->next;\\n'
    '\\t\\t\\t}\\n'
    '\\t\\t\\tif (curr->next != nullptr) {\\n'
    '\\t\\t\\t\\tcurr->next->prev = curr->prev;\\n'
    '\\t\\t\\t}\\n'
    '\\t\\t\\tcurr->prev = nullptr;'
)
src = src.replace(old, fixed, 1)
open(path, 'w', encoding='utf-8').write(src)
"
            "${_wkcore_pool_cpp}"
            RESULT_VARIABLE _patch_result
        )
        if(NOT _patch_result EQUAL 0)
            message(WARNING "Could not apply the MemoryPool::deallocate() patch (unexpected upstream content)")
        endif()
    else()
        message(WARNING "python3 not found - cannot apply the MemoryPool::deallocate() patch")
    endif()
else()
    message(WARNING "Workshop-Core MemoryPool.cpp not found at expected path ${_wkcore_pool_cpp} - the known allocator bug will NOT be patched")
endif()

set(_sctex_src_dir "${CMAKE_BINARY_DIR}/_deps/supercell-texture-src")
if(EXISTS "${_sctex_src_dir}")
    file(GLOB_RECURSE _sctex_headers "${_sctex_src_dir}/*.hpp" "${_sctex_src_dir}/*.h")
    foreach(_hdr ${_sctex_headers})
        file(READ "${_hdr}" _hdr_contents)
        string(FIND "${_hdr_contents}" "pragma region" _has_region)
        if(NOT _has_region EQUAL -1)
            string(REGEX REPLACE "#pragma region[^\n]*" "" _hdr_contents "${_hdr_contents}")
            string(REGEX REPLACE "#pragma endregion[^\n]*" "" _hdr_contents "${_hdr_contents}")
            file(WRITE "${_hdr}" "${_hdr_contents}")
        endif()
    endforeach()
else()
    message(WARNING "supercell-texture source not found at expected path ${_sctex_src_dir} - GCC may fail to build it")
endif()

find_package(ZLIB QUIET)
if(NOT ZLIB_FOUND)
    FetchContent_Declare(
        zlib-cmake
        URL https://github.com/jimmy-park/zlib-cmake/archive/main.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(zlib-cmake)
endif()

FetchContent_Declare(
    AnimateLib
    GIT_REPOSITORY https://github.com/sc-workshop/AnimateLib.git
    GIT_TAG main
)
FetchContent_MakeAvailable(AnimateLib)

include(WkLocale)
wk_include_locale()
