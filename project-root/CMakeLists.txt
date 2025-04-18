cmake_minimum_required(VERSION 3.16)
project(ForexArbitrage LANGUAGES CXX)

# ─────────── C++ Standard ───────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─────────── Output directories ───────────
# Single‑config generators (e.g. Makefiles, Ninja):
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY       ${CMAKE_BINARY_DIR})
# Multi‑config generators (e.g. Visual Studio):
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

# ─────────── Compiler‑specific flags ───────────
if (MSVC)
  # High warnings always
  add_compile_options(/W4)
  # Release optimizations
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
  add_compile_options(-O3 -Wall -Wextra -pedantic -march=native)
endif()

# ─────────── Includes ───────────
include_directories(${CMAKE_SOURCE_DIR}/include)

# ─────────── Sources ───────────
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)





# ─────────── Target ───────────
add_executable(arbitrage
    ${CMAKE_SOURCE_DIR}/main.cpp
    ${SRC_FILES}
)


add_custom_command(TARGET arbitrage
  PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data
    ${CMAKE_BINARY_DIR}/data
)