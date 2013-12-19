/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

import qbs 1.0

Product {
    id: libvpvl2
    property string libraryBuildDirectory: "build-" + qbs.buildVariant.toLowerCase()
    property string libraryInstallDirectory: libraryBuildDirectory + "/install-root"
    property string assimpLibrarySuffix: qbs.enableDebugCode ? "D" : ""
    property string nvFXLibrarySuffix: (cpp.architecture === "x86_64" ? "64" : "") + (qbs.enableDebugCode ? "D" : "")
    property var commonFiles: [
        "src/core/asset/*.cc",
        "src/core/base/*.cc",
        "src/core/internal/*.cc",
        "src/core/mvd/*.cc",
        "src/core/pmd2/*.cc",
        "src/core/pmx/*.cc",
        "src/core/vmd/*.cc",
        "src/engine/fx/*.cc",
        "src/engine/gl2/*.cc",
        "src/engine/nvfx/*.cc",
        "src/ext/Archive.cc",
        "src/ext/BaseApplicationContext.cc",
        "src/ext/StringMap.cc",
        "src/ext/World.cc",
        "src/ext/XMLProject.cc",
        "include/**/*.h",
        "vendor/nvFX/*.cc",
        "vendor/SOIL/*.c",
        "vendor/minizip-1.1/*.c",
        "vendor/tinyxml2-1.0.11/*.cpp"
    ]
    property var commonLibraries: [
        "AntTweakBar",
        "assimp" + assimpLibrarySuffix,
        "FxParser" + nvFXLibrarySuffix,
        "FxLibGL" + nvFXLibrarySuffix,
        "FxLib" + nvFXLibrarySuffix,
        "BulletSoftBody",
        "BulletDynamics",
        "BulletCollision",
        "LinearMath",
        "tbb",
        "z"
    ]
    type: qbs.buildVariant === "debug" ? "dynamiclibrary" : "staticlibrary"
    name: "vpvl2"
    version: "0.13.0"
    files: commonFiles
    cpp.defines: [
        "VPVL2_ENABLE_QT",
        "USE_FILE32API"
    ]
    cpp.includePaths: [
        "include",
        "vendor/cl-1.2",
        "vendor/nvFX",
        "vendor/SOIL",
        "vendor/minizip-1.1",
        "vendor/tinyxml2-1.0.11",
        libraryBuildDirectory + "/include",
        "../glm-src",
        "../bullet-src/" + libraryInstallDirectory + "/include/bullet",
        "../assimp-src/" + libraryInstallDirectory + "/include",
        "../nvFX-src/" + libraryInstallDirectory + "/include",
        "../zlib-src/" + libraryInstallDirectory + "/include",
        "../AntTweakBar-src/include",
        "../tbb-src/include"
    ]
    cpp.libraryPaths: [
        "../bullet-src/" + libraryInstallDirectory + "/lib",
        "../assimp-src/" + libraryInstallDirectory + "/lib",
        "../nvFX-src/" + libraryInstallDirectory + "/lib",
        "../zlib-src/" + libraryInstallDirectory + "/lib",
        "../AntTweakBar-src/lib",
        "../tbb-src/lib"
    ]
    Properties {
        condition: qbs.targetOS.contains("osx")
        type: qbs.buildVariant === "debug" ? "frameworkbundle" : "staticlibrary"
        cpp.dynamicLibraries: commonLibraries
        cpp.frameworks: [
            "OpenGL",
            "OpenCL"
        ]
    }
    Properties {
        condition: !qbs.targetOS.contains("osx")
        cpp.dynamicLibraries: commonLibraries.concat("GL")
    }
    Group {
        condition: qbs.targetOS.contains("osx")
        name: "OSX Extension"
        files: [ "src/engine/cl/*.cc" ]
    }
    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.cxxFlags: [ "/wd4068", "/wd4355", "/wd4819" ]
    }
    Depends { name: "cpp" }
    Depends {
        name: "Qt"
        submodules: [ "core", "gui" ]
    }
}

