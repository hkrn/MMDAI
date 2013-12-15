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

Application {
    id: VPVM
    property string libraryBuildDirectory: "build-" + qbs.buildVariant.toLowerCase()
    property string libraryInstallDirectory: libraryBuildDirectory + "/install-root"
    property string assimpLibrarySuffix: qbs.enableDebugCode ? "D" : ""
    property string nvFXLibrarySuffix: (cpp.architecture === "x86_64" ? "64" : "") + (qbs.enableDebugCode ? "D" : "")
    type: "application"
    name: "VPVM"
    version: "0.33.0"
    files: [
        "src/*.cc",
        "include/*.h",
        "licenses/licenses.qrc",
        "../libvpvl2/src/resources/resources.qrc"
    ]
    cpp.includePaths: [
        "include",
        "../libvpvl2/include",
        "../libvpvl2/" + libraryBuildDirectory + "/include",
        "../libvpvl2/vendor/cl-1.2",
        "../libvpvl2/vendor/nvFX",
        "../libvpvl2/vendor/tinyxml2-1.0.11",
        "../glog-src/" + libraryInstallDirectory + "/include",
        "../bullet-src/" + libraryInstallDirectory + "/include/bullet",
        "../assimp-src/" + libraryInstallDirectory + "/include",
        "../nvFX-src/" + libraryInstallDirectory + "/include",
        "../tbb-src/include",
        "../glm-src",
        "../alure-src/include",
        "../libgizmo-src/inc",
        "../AntTweakBar-src/include",
        "../openal-soft-src/" + libraryInstallDirectory + "/include",
        "../icu4c-src/" + libraryInstallDirectory + "/include"
    ]
    cpp.libraryPaths: [
        "../glog-src/" + libraryInstallDirectory + "/lib",
        "../bullet-src/" + libraryInstallDirectory + "/lib",
        "../assimp-src/" + libraryInstallDirectory + "/lib",
        "../nvFX-src/" + libraryInstallDirectory + "/lib",
        "../tbb-src/lib",
        "../libgizmo-src/" + libraryBuildDirectory,
        "../alure-src/" + libraryInstallDirectory + "/lib",
        "../openal-soft-src/" + libraryInstallDirectory + "/lib",
        "../icu4c-src/" + libraryInstallDirectory + "/lib",
        "../AntTweakBar-src/lib"
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
        "icuuc",
        "icui18n",
        "icudata",
        "glog",
        "tbb",
        "gizmo",
        "OpenAL",
        "alure-static",
        "AntTweakBar"
    ]
    cpp.frameworks: [
        "OpenGL",
        "OpenCL"
    ]
    Qt.quick.qmlDebugging: qbs.enableDebugCode
    Group {
        name: "Translation Resources"
        files: "translations/*.qm"
        qbs.install: true
    }
    Group {
        condition: qbs.buildVariant === "debug"
        name: "QML Resources"
        files: "qml/VPVM/*.qml"
        qbs.install: true
    }
    Depends { name: "cpp" }
    Depends { name: "vpvl2" }
    Depends {
        name: "Qt"
        submodules: [ "core", "gui", "widgets", "qml", "quick", "multimedia" ]
    }
}
