cmake_minimum_required(VERSION 3.29.20240416 FATAL_ERROR)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_CXX_MODULE_STD 1)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)

project(Argo LANGUAGES CXX)

option(ARGO_TESTS_ENABLE "Enable tests" OFF)
option(ARGO_BENCHMARKS_ENABLE "Enable benchmarks" OFF)
option(ARGO_CLANG_TIDY_ENABLE "Enable clang tidy" OFF)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})
  set(CMAKE_CXX_EXTENSIONS OFF)

  message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Winline")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wfloat-equal")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++") # Force to use
  # libc++

  message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")
  message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")
endif()

add_library(Argo)

file(GLOB ARGO_LIBRARY_SOURCES ${CMAKE_CURRENT_LIST_DIR}/Argo/*.cc)

target_sources(Argo PUBLIC FILE_SET CXX_MODULES FILES ${ARGO_LIBRARY_SOURCES})
target_compile_features(Argo PUBLIC cxx_std_23)
target_include_directories(Argo PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

if(${ARGO_CLANG_TIDY_ENABLE})
  find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy")
  if(NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "clang-tidy not found.")
  else()
    message(STATUS "Clang tidy: ${CLANG_TIDY_EXE}")
    set_target_properties(
      Argo PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p;${CMAKE_BINARY_DIR}")
  endif()
endif()

unset(ARGO_LIBRARY_SOURCES)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})

  add_executable(main ${CMAKE_CURRENT_LIST_DIR}/main.cc)
  target_link_libraries(main Argo)
  target_compile_options(main PRIVATE -O3)

  # ############################### TESTS ######################################
  if(${ARGO_TESTS_ENABLE})
    # Reset all optimization flags
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    FetchContent_MakeAvailable(googletest)

    file(GLOB ARGO_TEST_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tests/*.cc)

    # ############################## DEBUG TARGET ##############################
    enable_testing()
    add_executable(test-argo ${ARGO_TEST_SOURCES})

    target_link_libraries(test-argo PRIVATE GTest::gtest GTest::gtest_main
                                            GTest::gmock Argo)
    if(${ARGO_CLANG_TIDY_ENABLE})
      set_target_properties(
        test-argo PROPERTIES CXX_CLANG_TIDY
                             "${CLANG_TIDY_EXE};-p;${CMAKE_BINARY_DIR}")
    endif()
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
      target_compile_options(test-argo PRIVATE -fprofile-instr-generate
                                               -fcoverage-mapping)
      target_link_options(test-argo PRIVATE -fprofile-instr-generate
                          -fcoverage-mapping)
    endif()
    set_target_properties(test-argo PROPERTIES CXX_STANDARD 23)
    if(${ARGO_CLANG_TIDY_ENABLE})
      set_target_properties(
        test-argo PROPERTIES CXX_CLANG_TIDY
                             "clang-tidy;--quiet;-p;${CMAKE_BINARY_DIR}")
    endif()
    include(GoogleTest)
    gtest_discover_tests(test-argo)
  endif()

  # ############################## BENCHMARKS #################################
  if(${ARGO_BENCHMARKS_ENABLE})
    find_package(benchmark REQUIRED)
    file(GLOB ARGO_BENCHMARK_SOURCES ${CMAKE_CURRENT_LIST_DIR}/benchmarks/*.cc)

    find_package(CLI11 QUIET)
    if(${CLI11_FOUND})
      set(CLI11_LIBRARY CLI11::CLI11)
      add_compile_definitions(CLI11_FOUND=1)
    else()
      set(CLI11_LIBRARY "")
      message("CLI11 not found skipping")
    endif()

    find_package(argparse)
    if(${argparse_FOUND})
      set(argparse_LIBRARY argparse::argparse)
      add_compile_definitions(argparse_FOUND=1)
    else()
      set(argparse_LIBRARY "")
      message("argparse not found skipping")
    endif()

    foreach(target ${ARGO_BENCHMARK_SOURCES})
      get_filename_component(target_name ${target} NAME_WE)
      set(bench_target_name "bench-${target_name}")
      add_executable(${bench_target_name} ${target})
      target_link_libraries(${bench_target_name} benchmark::benchmark Argo
                            ${CLI11_LIBRARY})
      set_target_properties(${bench_target_name} PROPERTIES CXX_STANDARD 23)
      target_compile_options(${bench_target_name} PRIVATE -Wno-writable-strings
                                                          -O3)
    endforeach()
  endif()
endif()
