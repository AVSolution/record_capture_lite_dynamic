from conans import ConanFile, MSBuild

class RecordCaptureConan(ConanFile):
    name = "recordcapture"
    license = "MIT"
    url = "https://github.com/AVSolution/record_capture_lite_dynamic.git"
    settings = "os", "compiler", "build_type", "arch"
    requests = "RPC/0.0.3@bixin/stable"
    generators = "visual_studio"
    exports_sources = "recordcapture*","!*.vs"
	
    def build(self):
        msbuild = MSBuild(self)
        msbuild.build("recordcapture/recordcapture.vcxproj")

    def package(self):
        self.copy("capture.h", dst="include/recordcapture", src="recordcapture")
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.pdb", dst="bin", excludes="*vc141.pdb",keep_path=False)
        self.copy("*.lib", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["recordcapture"]
