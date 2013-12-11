import qbs 1.0

Product {
	property string libraryBuildDirectory: "build-" + qbs.buildVariant.toLowerCase()
	property string libraryInstallDirectory: libraryBuildDirectory + "/install-root"
	property string assimpLibrarySuffix: qbs.enableDebugCode ? "D" : ""
	property string nvFXLibrarySuffix: (cpp.architecture === "x86_64" ? "64" : "") + (qbs.enableDebugCode ? "D" : "")
	type: "frameworkbundle"
	name: "vpvl2qt"
	files: [
	    "src/core/asset/*.cc",
	    "src/core/base/*.cc",
	    "src/core/internal/*.cc",
	    "src/core/mvd/*.cc",
	    "src/core/pmd2/*.cc",
	    "src/core/pmx/*.cc",
	    "src/core/vmd/*.cc",
	    "src/engine/cl/*.cc",
	    "src/engine/fx/*.cc",
	    "src/engine/gl2/*.cc",
	    "src/engine/nvfx/*.cc",
	    "vendor/nvFX/*.cc",
	    "vendor/tinyxml2-1.0.11/*.cpp"
	]
	cpp.defines: [ "VPVL2_ENABLE_QT" ]
	cpp.includePaths: [
	    "include",
	    "vendor/cl-1.2",
	    "vendor/nvFX",
	    "vendor/tinyxml2-1.0.11",
	    libraryBuildDirectory + "/include",
	    "../glog-src/" + libraryInstallDirectory + "/include",
	    "../bullet-src/" + libraryInstallDirectory + "/include/bullet",
	    "../assimp-src/" + libraryInstallDirectory + "/include",
	    "../nvFX-src/" + libraryInstallDirectory + "/include",
	    "../tbb-src/include",
	]
	cpp.libraryPaths: [
	    "../glog-src/" + libraryInstallDirectory + "/lib",
	    "../bullet-src/" + libraryInstallDirectory + "/lib",
	    "../assimp-src/" + libraryInstallDirectory + "/lib",
	    "../nvFX-src/" + libraryInstallDirectory + "/lib",
	    "../tbb-src/lib"
	]
	cpp.dynamicLibraries: [
	    "LinearMath",
	    "BulletCollision",
	    "BulletDynamics",
	    "BulletSoftBody",
	    "assimp" + assimpLibrarySuffix,
	    "FxParser" + nvFXLibrarySuffix,
	    "FxLib" + nvFXLibrarySuffix,
	    "FxLibGL" + nvFXLibrarySuffix,
	    "glog",
	    "tbb"
	]
	cpp.frameworks: [
	    "OpenGL",
	    "OpenCL"
	]
	version: "0.13.0"
	Depends { name: "cpp" }
	Depends { name: "Qt"; submodules: [ "core", "gui" ] }
}

