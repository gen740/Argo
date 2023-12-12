cmake_minimum_required(VERSION 3.28)

project(Argo CXX)

set(CMAKE_CXX_STANDARD 23)

option(ARGO_TESTS_ENABLE "Enable tests" OFF)
option(ARGO_BENCHMARKS_ENABLE "Enable benchmarks" OFF)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})
  # Setting Build Type
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
  endif()

  # set release flag
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Winline")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wfloat-equal")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
endif()

add_library(Argo)

file(GLOB ARGO_LIBRARY_SOURCES ${CMAKE_CURRENT_LIST_DIR}/Argo/*.cc)

target_sources(Argo PUBLIC FILE_SET CXX_MODULES FILES ${ARGO_LIBRARY_SOURCES})
target_compile_features(Argo PUBLIC cxx_std_23)

unset(ARGO_LIBRARY_SOURCES)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})
  add_executable(main ${CMAKE_CURRENT_LIST_DIR}/main.cc)
  target_link_libraries(main Argo)
  target_compile_options(main PRIVATE -save-temps -O3 -ftime-trace)


  ################################ TESTS ######################################
  if(${ARGO_TESTS_ENABLE})
    # Reset all optimization flags
    set(CMAKE_CXX_FLAGS_RELEASE "")
    set(CMAKE_CXX_FLAGS_DEBUG "")

    find_package(GTest REQUIRED)
    file(GLOB ARGO_TEST_SOURCES ${CMAKE_CURRENT_LIST_DIR}/tests/*.cc)

    ############################### DEBUG TARGET ##############################
    add_executable(test-argo-debug ${ARGO_TEST_SOURCES})
    target_link_libraries(test-argo-debug GTest::GTest GTest::Main GTest::gmock
                          Argo)
    set_target_properties(test-argo-debug PROPERTIES CXX_STANDARD 23)
    target_compile_options(test-argo-debug PRIVATE -Wno-writable-strings
                                                   -DDEBUG_BUILD -O1 -g)

    ############################### RELEASE TARGET ############################
    add_executable(test-argo-release ${ARGO_TEST_SOURCES})
    target_link_libraries(test-argo-release GTest::GTest GTest::Main
                          GTest::gmock Argo)
    set_target_properties(test-argo-release PROPERTIES CXX_STANDARD 23)
    target_compile_options(test-argo-release PRIVATE -Wno-writable-strings -O3)
  endif()

  ############################### BENCHMARKS #################################
  if(${ARGO_BENCHMARKS_ENABLE})
    set(CMAKE_CXX_FLAGS_RELEASE "")
    set(CMAKE_CXX_FLAGS_DEBUG "")

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

      ############################## DEBUG TARGET #############################
      set(debug_target_name "bench-${target_name}-debug")
      add_executable(${debug_target_name} ${target})
      target_link_libraries(${debug_target_name} benchmark::benchmark Argo
                            ${CLI11_LIBRARY} ${argparse_LIBRARY})
      set_target_properties(${debug_target_name} PROPERTIES CXX_STANDARD 23)
      target_compile_options(${debug_target_name} PRIVATE -Wno-writable-strings
                                                          -DDEBUG_BUILD)
      ############################## RELEASE TARGET ###########################
      set(release_target_name "bench-${target_name}-release")
      add_executable(${release_target_name} ${target})
      target_link_libraries(${release_target_name} benchmark::benchmark Argo
                            ${CLI11_LIBRARY})
      set_target_properties(${release_target_name} PROPERTIES CXX_STANDARD 23)
      target_compile_options(${release_target_name}
                             PRIVATE -Wno-writable-strings -O3 -Os)
    endforeach()
  endif()
endif()
