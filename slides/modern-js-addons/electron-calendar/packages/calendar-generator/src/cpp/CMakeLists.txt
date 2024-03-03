cmake_minimum_required(VERSION 3.28)

project(calendar-generator)

# Include Node-API wrappers
execute_process(
  COMMAND node -p "require('node-addon-api').include"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(
  REGEX
  REPLACE "[\r\n\"]"
          ""
          NODE_ADDON_API_DIR
          ${NODE_ADDON_API_DIR})

# get supported Node-API version
file(READ ${CMAKE_SOURCE_DIR}/../../package.json PACKAGE_JSON)
string(JSON BINARY GET "${PACKAGE_JSON}" binary)
string(JSON NAPI_VERSIONS GET "${BINARY}" napi_versions)
string(JSON NODE_API_VERSION GET "${NAPI_VERSIONS}" 0)
if (NOT NODE_API_VERSION)
  message(FATAL_ERROR "No supported Node-API version found in package.json")
endif()

message(STATUS "Using Node-API from: ${NODE_ADDON_API_DIR} with version: ${NODE_API_VERSION}")
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(range-v3 CONFIG REQUIRED)

add_library(calendar STATIC)
target_sources(calendar
  PRIVATE
    FILE_SET CXX_MODULES FILES
      calendar.cxx closure.cxx concat.cxx
)
target_link_libraries(calendar PUBLIC range-v3::range-v3)

add_library(${PROJECT_NAME} SHARED ${CMAKE_JS_SRC} interface.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE NAPI_VERSION=${NODE_API_VERSION})
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE 
  calendar
  ${CMAKE_JS_LIB})

add_executable(${PROJECT_NAME}-cli main.cpp)
target_link_libraries(${PROJECT_NAME}-cli PRIVATE calendar)

add_custom_target(prebuild ALL
                  COMMAND pkg-prebuilds-copy --baseDir .
                          --source $<PATH:RELATIVE_PATH,$<TARGET_FILE:${PROJECT_NAME}>,$<PATH:NORMAL_PATH,${CMAKE_SOURCE_DIR}>>
                          --name ${PROJECT_NAME} --napi_version ${NODE_API_VERSION})
