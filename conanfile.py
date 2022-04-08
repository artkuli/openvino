from conans import ConanFile, CMake, tools


class OpenvinoConan(ConanFile):
    name = "openvino"
    version = "2022.1"
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of Openvino here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = ["cmake", "CMakeToolchain", "CMakeDeps"]
    exports_sources = ["cmake/*", "docs/*", "licensing/*", "ngraph/*", "samples/*", "scripts/*", "src/*", "tests/*", "thirdparty/*", "tools/*", "CMakeLists.txt", "conanfile.txt"]

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def requirements(self):
        self.requires("onnx/1.11.0")

    def configure_cmake(self):
        self.cmake = CMake(self)
        self.cmake.verbose = True
        self.cmake.definitions["ENABLE_INTEL_CPU"] = "ON"
        self.cmake.definitions["ENABLE_INTEL_CPU"]="ON" 
        self.cmake.definitions["ENABLE_INTEL_GPU"]="OFF" 
        self.cmake.definitions["ENABLE_INTEL_GNA"]="OFF" 
        self.cmake.definitions["ENABLE_INTEL_MYRIAD"]="OFF" 
        self.cmake.definitions["ENABLE_OPENCV"]="OFF" 
        self.cmake.definitions["ENABLE_TESTS"]="OFF" 
        self.cmake.definitions["ENABLE_BEH_TESTS"]="OFF" 
        self.cmake.definitions["ENABLE_FUNCTIONAL_TESTS"]="OFF" 
        self.cmake.definitions["ENABLE_PROFILING_ITT"]="OFF" 
        self.cmake.definitions["ENABLE_SAMPLES"]="OFF" 
        self.cmake.definitions["ENABLE_PYTHON"]="OFF" 
        self.cmake.definitions["PYTHON_EXECUTABLE"] ="/usr/bin/python3" 
        self.cmake.definitions["ENABLE_CPPLINT"]="OFF" 
        self.cmake.definitions["ENABLE_NCC_STYLE"]="OFF" 
        self.cmake.definitions["ENABLE_OV_PADDLE_FRONTEND"]="OFF" 
        self.cmake.definitions["ENABLE_OV_TF_FRONTEND"]="OFF" 
        self.cmake.definitions["ENABLE_TESTS"]="OFF"
        self.cmake.definitions["CMAKE_EXPORT_NO_PACKAGE_REGISTRY"] = "OFF"
        self.cmake.definitions["ENABLE_TEMPLATE"] = "OFF"
        self.cmake.definitions["ENABLE_INTEL_MYRIAD_COMMON"] = "OFF"
        self.cmake.definitions["CMAKE_CXX_FLAGS"] = "-Wno-error=undef -Wno-error=suggest-override"
        self.cmake.definitions["CMAKE_C_FLAGS"] = "-Wno-error=undef -Wno-error=suggest-override"
        self.cmake.configure()
        # return cmake


    def build(self):
        self.run('echo ' + self.source_folder)
        self.run('pwd; ls')
        self.configure_cmake()
        # print("source_folder=", self.source_folder)
        # cmake.configure()
        self.cmake.build()

    def package(self):
        self.cmake.install()
        # self.copy("*.h", dst="include", src="src")
        # self.copy("*hello.lib", dst="lib", keep_path=False)
        # self.copy("*.dll", dst="bin", keep_path=False)
        # self.copy("*.so", dst="lib", keep_path=False)
        # self.copy("*.dylib", dst="lib", keep_path=False)
        # self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["openvino"]


