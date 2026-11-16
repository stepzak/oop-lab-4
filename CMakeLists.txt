cmake_minimum_required(VERSION 3.20)
project(oop_lab_4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(SANITIZER_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer)
    else()
        message(FATAL_ERROR "ENABLE_SANITIZERS is supported only with GCC and Clang")
    endif()
endif()

add_library(polygons_lib STATIC
        src/polygons.cpp
)
target_include_directories(polygons_lib PUBLIC include)
target_compile_options(polygons_lib PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_SANITIZERS)
    target_compile_options(polygons_lib PRIVATE ${SANITIZER_FLAGS})
    target_link_options(polygons_lib INTERFACE ${SANITIZER_FLAGS})
endif()

add_executable(oop_lab_4 main.cpp)
target_link_libraries(oop_lab_4 PRIVATE polygons_lib)
target_compile_options(oop_lab_4 PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_SANITIZERS)
    target_compile_options(oop_lab_4 PRIVATE ${SANITIZER_FLAGS})
    target_link_options(oop_lab_4 PRIVATE ${SANITIZER_FLAGS})
endif()

include(CTest)

if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(oop_lab_4_tests tests/polygons_test.cpp)
    target_link_libraries(oop_lab_4_tests PRIVATE polygons_lib GTest::gtest_main)
    target_compile_options(oop_lab_4_tests PRIVATE -Wall -Wextra -Wpedantic)

    if(ENABLE_SANITIZERS)
        target_compile_options(oop_lab_4_tests PRIVATE ${SANITIZER_FLAGS})
        target_link_options(oop_lab_4_tests PRIVATE ${SANITIZER_FLAGS})
    endif()

    add_executable(small_vec_tests tests/small_vec_test.cpp)
    target_include_directories(small_vec_tests PRIVATE include)
    target_link_libraries(small_vec_tests PRIVATE GTest::gtest_main)
    target_compile_options(small_vec_tests PRIVATE -Wall -Wextra -Wpedantic)

    if(ENABLE_SANITIZERS)
        target_compile_options(small_vec_tests PRIVATE ${SANITIZER_FLAGS})
        target_link_options(small_vec_tests PRIVATE ${SANITIZER_FLAGS})
    endif()

    include(GoogleTest)
    gtest_discover_tests(oop_lab_4_tests)
    gtest_discover_tests(small_vec_tests)
endif()
