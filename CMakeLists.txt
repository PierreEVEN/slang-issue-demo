cmake_minimum_required(VERSION 3.8)

project(SlangIssueDemo)

set(ARG_IGNORE_FAILURE OFF CACHE BOOL OFF)
set(SLANG_ENABLE_GFX OFF CACHE BOOL OFF)
set(SLANG_ENABLE_SLANGD OFF CACHE BOOL OFF)
set(SLANG_ENABLE_SLANGRT OFF CACHE BOOL OFF)
set(SLANG_ENABLE_SLANG_GLSLANG OFF CACHE BOOL OFF)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL OFF)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL OFF)
#set(SLANG_VERSION_FULL "0.0.0" CACHE STRING "0.0.0")
add_subdirectory(slang)

file(GLOB_RECURSE SOURCES src/**)
add_executable(SlangIssueDemo ${SOURCES})
configure_project(SlangIssueDemo ${SOURCES})
target_link_libraries(SlangIssueDemo PUBLIC slang)
target_include_directories(SlangIssueDemo PRIVATE src)