cmake_minimum_required(VERSION 3.20)
project(oop_lab_4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(polygons_lib STATIC
        src/polygons.cpp
)
target_include_directories(polygons_lib PUBLIC include)
target_compile_options(polygons_lib PRIVATE -Wall -Wextra -Wpedantic)

add_executable(oop_lab_4 main.cpp)
target_link_libraries(oop_lab_4 PRIVATE polygons_lib)
target_compile_options(oop_lab_4 PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)

if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(oop_lab_4_tests tests/polygons_test.cpp)
    target_link_libraries(oop_lab_4_tests PRIVATE polygons_lib GTest::gtest_main)
    target_compile_options(oop_lab_4_tests PRIVATE -Wall -Wextra -Wpedantic)

    include(GoogleTest)
    gtest_discover_tests(oop_lab_4_tests)
endif()
