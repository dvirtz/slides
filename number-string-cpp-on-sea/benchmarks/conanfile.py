from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps

class NumberStringBenchmarks(ConanFile):
  name = 'number-string-conversion-benchmarks'
  requires = [
    'fmt/8.1.1',
    'scnlib/1.1.2',
    'benchmark/1.6.1'
  ]
  settings = "os", "compiler", "arch", "build_type"

  def generate(self):
    deps = CMakeDeps(self)
    deps.generate()

    toolchain = CMakeToolchain(self)
    toolchain.variables['CMAKE_CXX_STANDARD'] = '20'
    toolchain.generate()

  def build(self):
    cmake = CMake(self)
    cmake.configure()
    cmake.build()
