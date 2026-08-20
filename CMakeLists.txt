cmake_minimum_required(VERSION 3.20)
project(DupliGuard VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dupliguard_core STATIC core/src/sha256.c)
target_include_directories(dupliguard_core PUBLIC core/include)

add_executable(dupliguard cli/src/main.cpp cli/src/scanner.cpp)
target_include_directories(dupliguard PRIVATE cli/include)
target_link_libraries(dupliguard PRIVATE dupliguard_core)

if(MSVC)
  target_compile_options(dupliguard_core PRIVATE /W4 /WX)
  target_compile_options(dupliguard PRIVATE /W4 /WX)
else()
  target_compile_options(dupliguard_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(dupliguard PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(test_sha256 tests/test_sha256.c)
  target_link_libraries(test_sha256 PRIVATE dupliguard_core)
  add_test(NAME sha256_vectors COMMAND test_sha256)
endif()

find_package(Java 17 COMPONENTS Development QUIET)
if(Java_FOUND)
  add_custom_target(dupliguard_desktop
    COMMAND ${Java_JAVAC_EXECUTABLE} -d ${CMAKE_BINARY_DIR}/java
            ${CMAKE_SOURCE_DIR}/desktop/src/main/java/io/dupliguard/DesktopApp.java
    COMMENT "Compiling Java desktop application")
endif()
