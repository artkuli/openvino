from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout


class OpenvinoConan(ConanFile):
    name = "openvino"
    version = "0.1"

    # Optional metadata
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of OpenVINO here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = ["CMakeToolchain", "CMakeDeps", "cmake_paths"]

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "cmake/*",  "ngraph/*", "scripts/*", "src/*", "thirdparty/*", "tools/*", "CMakeLists.txt"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.definitions["ENABLE_INTEL_CPU"] = "ON"
        cmake.definitions["ENABLE_INTEL_CPU"]="ON" 
        cmake.definitions["ENABLE_INTEL_GPU"]="OFF" 
        cmake.definitions["ENABLE_INTEL_GNA"]="OFF" 
        cmake.definitions["ENABLE_INTEL_MYRIAD"]="OFF" 
        cmake.definitions["ENABLE_OPENCV"]="OFF" 
        cmake.definitions["ENABLE_TESTS"]="OFF" 
        cmake.definitions["ENABLE_BEH_TESTS"]="OFF" 
        cmake.definitions["ENABLE_FUNCTIONAL_TESTS"]="OFF" 
        cmake.definitions["ENABLE_PROFILING_ITT"]="OFF" 
        cmake.definitions["ENABLE_SAMPLES"]="OFF" 
        cmake.definitions["ENABLE_PYTHON"]="OFF" 
        cmake.definitions["PYTHON_EXECUTABLE"] ="/usr/bin/python3" 
        cmake.definitions["ENABLE_CPPLINT"]="OFF" 
        cmake.definitions["ENABLE_NCC_STYLE"]="OFF" 
        cmake.definitions["ENABLE_OV_PADDLE_FRONTEND"]="OFF" 
        cmake.definitions["ENABLE_OV_TF_FRONTEND"]="OFF" 
        cmake.definitions["ENABLE_TESTS"]="OFF"
        cmake.definitions["CMAKE_EXPORT_NO_PACKAGE_REGISTRY"] = "OFF"
        cmake.definitions["ENABLE_TEMPLATE"] = "OFF"
        cmake.definitions["ENABLE_INTEL_MYRIAD_COMMON"] = "OFF"
        cmake.definitions["CMAKE_CXX_FLAGS"] = "-Wno-error=undef -Wno-error=suggest-override"
        cmake.definitions["CMAKE_C_FLAGS"] = "-Wno-error=undef -Wno-error=suggest-override"
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["openvino"]
        self.cpp_info.build_modules["cmake_find_package"].append("cmake/Findopenvino.cmake")
        self.cpp_info.includedirs = ["runtime/include", "runtime/include/ie", "runtime/include/ngraph", "runtime/include/openvino"]
        self.cpp_info.libdirs = ["runtime/lib/intel64", "runtime/3rdparty/tbb_bind_2_5/lib"]