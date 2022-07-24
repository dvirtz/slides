cmake_minimum_required(VERSION 3.10)

project(NumberStringConversions)

find_package(fmt CONFIG REQUIRED)
find_package(scn CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

foreach(bench atod-digit atoi dtoa-random itoa)
  add_executable(${bench} ${bench}.cpp)
  target_link_libraries(${bench} PRIVATE fmt::fmt scn::scn benchmark::benchmark)
endforeach()
