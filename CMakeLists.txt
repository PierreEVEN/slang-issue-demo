cmake_minimum_required(VERSION 3.8)

project(SlangIssueDemo)

if (MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/bin")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif (MSVC)

set(ARG_IGNORE_FAILURE OFF CACHE BOOL OFF)
set(SLANG_ENABLE_GFX OFF CACHE BOOL OFF)
set(SLANG_ENABLE_SLANGD OFF CACHE BOOL OFF)
set(SLANG_ENABLE_SLANGRT OFF CACHE BOOL OFF)
set(SLANG_ENABLE_SLANG_GLSLANG OFF CACHE BOOL OFF)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL OFF)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL OFF)
add_subdirectory(slang)

file(GLOB_RECURSE SOURCES src/**)
add_executable(SlangIssueDemo ${SOURCES})
target_compile_features(SlangIssueDemo  PRIVATE cxx_std_17)
target_link_libraries(SlangIssueDemo PUBLIC slang)
target_include_directories(SlangIssueDemo PRIVATE src)