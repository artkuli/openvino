from conans import ConanFile, CMake


class OpenvinoConan(ConanFile):
    name = "openvino"
    version = "2022.3"

    # metadata
    license = "Apache 2.0"
    author = "Intel Corporation"
    url = "https://github.com/openvinotoolkit/openvino"
    description = "OpenVINO™ is an open-sorce toolkit for optimizing and deploying AI inference"
    topics = ("deep-learning", "artificial-intelligence", "framework")

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = ["cmake", "cmake_find_package", "CMakeDeps", "cmake_paths"]

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "cmake/*",  "ngraph/*", "scripts/*", "src/*", "thirdparty/*", "tools/*", "samples/*", "docs/*", "licensing/*", "tests/*", "CMakeLists.txt"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def requirements(self):
        # Don't need nlohmann_json because json-schema-validator requires it
        self.requires("json-schema-validator/2.1.0")
        self.requires("pugixml/1.11")
        self.requires("zlib/1.2.12")
        self.requires("onnx/1.11.0")

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.definitions["ENABLE_INTEL_CPU"] = "ON"
        cmake.definitions["ENABLE_INTEL_GPU"]="OFF" 
        cmake.definitions["ENABLE_INTEL_GNA"]="OFF" 
        cmake.definitions["ENABLE_INTEL_MYRIAD"]="OFF" 
        cmake.definitions["ENABLE_OPENCV"]="OFF" 
        cmake.definitions["ENABLE_TESTS"]="OFF" 
        cmake.definitions["ENABLE_BEH_TESTS"]="OFF" 
        cmake.definitions["ENABLE_FUNCTIONAL_TESTS"]="OFF" 
        cmake.definitions["ENABLE_PROFILING_ITT"]="OFF" 
        cmake.definitions["ENABLE_SAMPLES"]="ON" 
        cmake.definitions["ENABLE_PYTHON"]="OFF" 
        cmake.definitions["ENABLE_CPPLINT"]="OFF" 
        cmake.definitions["ENABLE_NCC_STYLE"]="OFF" 
        cmake.definitions["ENABLE_OV_PADDLE_FRONTEND"]="ON" 
        cmake.definitions["ENABLE_OV_TF_FRONTEND"]="ON" 
        cmake.definitions["ENABLE_OV_ONNX_FRONTEND"]="ON" 
        cmake.definitions["CMAKE_EXPORT_NO_PACKAGE_REGISTRY"] = "OFF"
        cmake.definitions["ENABLE_TEMPLATE"] = "OFF"
        cmake.definitions["ENABLE_INTEL_MYRIAD_COMMON"] = "OFF"
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