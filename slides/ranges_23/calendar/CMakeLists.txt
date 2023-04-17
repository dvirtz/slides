cmake_minimum_required(VERSION 3.10)

project(ranges_23)

################# dependencies #################

set(CONAN_CMAKE_VERSION 0.18.1)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan-${CONAN_CMAKE_VERSION}.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/${CONAN_CMAKE_VERSION}/conan.cmake"
       "${CMAKE_BINARY_DIR}/conan-${CONAN_CMAKE_VERSION}.cmake" TLS_VERIFY ON)
endif()

set_directory_properties(PROPERTIES CMAKE_CONFIGURE_DEPENDS conanfile.txt)

include(${CMAKE_BINARY_DIR}/conan-${CONAN_CMAKE_VERSION}.cmake)

foreach(TYPE ${CMAKE_CONFIGURATION_TYPES})
  # set(CMAKE_BUILD_TYPE ${TYPE})
  conan_cmake_autodetect(settings BUILD_TYPE ${TYPE})

  conan_cmake_install(
    PATH_OR_REFERENCE
    ${CMAKE_SOURCE_DIR}
    BUILD
    missing
    SETTINGS
    ${settings})
endforeach()

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(fmt REQUIRED CONFIG)

################################################

add_executable(calendar calendar.cpp)

target_compile_features(calendar PRIVATE cxx_std_23)
target_compile_definitions(calendar PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_USE_JOIN_VIEW_INPUT_RANGE>)
target_link_libraries(calendar PRIVATE fmt::fmt)
