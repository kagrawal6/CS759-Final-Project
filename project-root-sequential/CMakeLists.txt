cmake_minimum_required(VERSION 3.10)
project(ForexArbitrage LANGUAGES CXX)

# 1) C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 2) Common warnings
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra)
endif()

# 3) MSVC: only optimize in Release
if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} /O2"
  )
else()
  # On GCC/Clang always optimize
  add_compile_options(-O3 -march=native)
endif()

# 4) OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# 5) Includes
include_directories(${CMAKE_SOURCE_DIR}/include)

# 6) Sources
file(GLOB SRC
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)
list(APPEND SRC "${CMAKE_SOURCE_DIR}/main.cpp")

# 7) Target
add_executable(arbitrage ${SRC})

# 8) Place the exe in build/
set_target_properties(arbitrage PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# 9) Link OpenMP
target_link_libraries(arbitrage PUBLIC OpenMP::OpenMP_CXX)
