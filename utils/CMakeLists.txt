cmake_minimum_required(VERSION 3.14)
project(utils)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# googletest setup

enable_testing()

add_executable(
  utils_unittests
  NoiseGate_unittests.cc
  AREnvelope_unittests.cc
)
target_link_libraries(
  utils_unittests
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(utils_unittests)
