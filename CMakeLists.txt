cmake_minimum_required(VERSION 3.22)
project(sc2fla LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
include(cmake/Dependencies.cmake)

file(GLOB_RECURSE SC2FLA_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(sc2fla ${SC2FLA_SOURCES})

target_include_directories(sc2fla PRIVATE src)

target_link_libraries(sc2fla PRIVATE
    supercell::flash
    Adobe::AnimateLib
)

if(MSVC)
    target_compile_options(sc2fla PRIVATE /utf-8)
else()
    target_compile_options(sc2fla PRIVATE -Wno-unused-parameter)
endif()

if(MINGW)
    target_link_options(sc2fla PRIVATE -Wl,--allow-multiple-definition)
endif()

install(TARGETS sc2fla RUNTIME DESTINATION bin)
