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
import qbs.File
import qbs.FileInfo
import qbs.TextFile
import "VPVM.qbs.js" as vpvm

Application {
    id: VPVM
    property string applicationBundlePath: "VPVM.app/Contents"
    property string libraryBuildDirectory: "build-" + qbs.buildVariant.toLowerCase()
    property string libraryInstallDirectory: libraryBuildDirectory + "/install-root"
    property string debugLibrarySuffix: qbs.enableDebugCode ? "d" : ""
    property string assimpLibrarySuffix: qbs.toolchain.contains("msvc") ? "" : debugLibrarySuffix.toUpperCase()
    property string nvFXLibrarySuffix: (cpp.architecture === "x86_64" ? "64" : "") + debugLibrarySuffix.toUpperCase()
    property var commonLibraries: [
        "AntTweakBar",
        "assimp" + assimpLibrarySuffix,
        "FxParser" + nvFXLibrarySuffix,
        "FxLibGL" + nvFXLibrarySuffix,
        "FxLib" + nvFXLibrarySuffix,
        "BulletSoftBody",
        "BulletDynamics",
        "BulletCollision",
        "LinearMath"
    ]
    property var commonIncludePaths: [
        "include",
        "../libvpvl2/include",
        "../libvpvl2/" + libraryBuildDirectory + "/include",
        "../libvpvl2/vendor/cl-1.2",
        "../libvpvl2/vendor/nvFX",
        "../libvpvl2/vendor/tinyxml2-1.0.11",
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
    property var commonFiles: [
        "src/*.cc",
        "include/*.h",
        "licenses/licenses.qrc",
        "../libvpvl2/src/resources/resources.qrc"
    ]
    property var requiredSubmodules: [
        "core", "gui", "widgets", "qml", "quick", "multimedia", "network"
    ]
    type: (qbs.targetOS.contains("osx") && !qbs.enableDebugCode) ? "applicationbundle" : "application"
    name: "VPVM"
    version: {
        var file = new TextFile(sourceDirectory + "/../libvpvl2/CMakeLists.txt", TextFile.ReadOnly), v = {}
        while (!file.atEof()) {
            var line = file.readLine()
            if (line.match(/(VPVL2_VERSION_\w+)\s+(\d+)/)) {
                v[RegExp.$1] = RegExp.$2
            }
        }
        return [ v["VPVL2_VERSION_MAJOR"], v["VPVL2_VERSION_COMPAT"], "1" ].join(".")
    }
    files: commonFiles
    cpp.defines: {
        var defines = [ "VPVL2_ENABLE_QT" ]
        if (qbs.enableDebugCode && qbs.toolchain.contains("msvc")) {
            defines.push("BUILD_SHARED_LIBS")
        }
        return defines
    }
    cpp.includePaths: commonIncludePaths
    cpp.libraryPaths: [
        "../AntTweakBar-src/lib/" + (qbs.toolchain.contains("msvc") && qbs.enableDebugCode ? "debug" : ""),
        "../tbb-src/lib",
        "../bullet-src/" + libraryInstallDirectory + "/lib",
        "../assimp-src/" + libraryInstallDirectory + "/lib",
        "../nvFX-src/" + libraryInstallDirectory + "/lib",
        "../alure-src/" + libraryInstallDirectory + "/lib",
        "../openal-soft-src/" + libraryInstallDirectory + "/lib",
        "../icu4c-src/" + libraryInstallDirectory + "/lib",
        "../zlib-src/" + libraryInstallDirectory + "/lib"
    ]
    Qt.quick.qmlDebugging: qbs.enableDebugCode
    Group {
        name: "Application"
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: qbs.targetOS.contains("osx") ? FileInfo.joinPaths(applicationBundlePath, "MacOS") : ""
    }
    Group {
        name: "Application Translation Resources"
        files: [
            "translations/*.qm",
            Qt.core.binPath + "/../translations/qt*.qm",
        ]
        qbs.install: true
        qbs.installDir: qbs.targetOS.contains("osx") ? FileInfo.joinPaths(applicationBundlePath, "Resources", "translations") : "translations"
    }
    Group {
        condition: qbs.buildVariant === "release"
        name: "Application Resources for Release Build"
        files: {
            var files = [ "qml/VPVM.qrc" ]
            if (qbs.targetOS.contains("windows")) {
                files.push("qt/win32.qrc")
            }
            else if (qbs.targetOS.contains("osx")) {
                files.push("qt/osx.qrc")
            }
            else if (qbs.targetOS.contains("linux")) {
                files.push("qt/linux.qrc")
            }
            if (!qbs.toolchain.contains("msvc")) {
                files.push("libav/libav.qrc")
            }
            return files
        }
    }
    Group {
        name: "Application Resources for Debug Build"
        files: {
            var files = [ "qml/VPVM/*" ]
            if (!qbs.toolchain.contains("msvc")) {
                files.push("libav/libav.qrc")
            }
            return files
        }
        qbs.install: qbs.buildVariant === "debug"
        qbs.installDir: qbs.targetOS.contains("osx") ? FileInfo.joinPaths(applicationBundlePath, "Resources", "qml") : "qml"
    }
    Properties {
        condition: qbs.targetOS.contains("osx")
        cpp.frameworks: [ "AppKit", "OpenGL", "OpenCL" ]
        cpp.infoPlistFile: "qt/osx/Info.plist"
        cpp.infoPlist: ({
                            "CFBundleVersion": version,
                            "CFBundleShortVersionString": version
                        })
        cpp.dynamicLibraries: commonLibraries.concat([ "alure-static", "openal", "tbb", "z" ])
    }
    Properties {
        condition: !qbs.targetOS.contains("osx") && !qbs.targetOS.contains("windows")
        cpp.dynamicLibraries: commonLibraries.concat([ "alure-static", "openal", "Xext", "X11", "tbb", "z", "GL"])
        cpp.rpaths: [ "$ORIGIN/lib", "$ORIGIN" ]
        cpp.positionIndependentCode: true
    }
    Properties {
        condition: qbs.toolchain.contains("mingw")
        cpp.includePaths: commonIncludePaths.concat([
                                                        "../alure-src/include/AL",
                                                        "../openal-soft-src/" + libraryInstallDirectory + "/include/AL"
                                                    ])
        cpp.dynamicLibraries: commonLibraries.concat([
                                                         "alure32-static",
                                                         "OpenAL32",
                                                         "OpenGL32",
                                                         "zlibstatic" + debugLibrarySuffix
                                                     ])
    }
    Properties {
        condition: qbs.toolchain.contains("msvc")
        consoleApplication: false
        cpp.cxxFlags: [ "/wd4068", "/wd4355", "/wd4819" ]
        cpp.includePaths: commonIncludePaths.concat([
                                                        "../alure-src/include/AL",
                                                        "../openal-soft-src/" + libraryInstallDirectory + "/include/AL"
                                                    ])
        cpp.dynamicLibraries: commonLibraries.concat([
                                                         "alure32-static",
                                                         "OpenAL32",
                                                         "libGLESv2" + debugLibrarySuffix,
                                                         "libEGL" + debugLibrarySuffix,
                                                         "zlibstatic" + debugLibrarySuffix
                                                     ])
    }
    Group {
        condition: !qbs.targetOS.contains("osx") && !qbs.targetOS.contains("windows")
        name: "Application Depending Libraries"
        files: {
            var found = vpvm.findLibraries(commonLibraries.concat([ "openal", "tbb", "z" ]), cpp.libraryPaths, ".so*")
            if (qbs.targetOS.contains("linux")) {
                requiredSubmodules.push("dbus")
            }
            for (var i in requiredSubmodules) {
                var requiredSubmodule = requiredSubmodules[i]
                var name = requiredSubmodule.toUpperCase().charAt(0) + requiredSubmodule.substring(1)
                found.push(FileInfo.joinPaths(Qt.core.libPath, "libQt5" + name + ".so.5*"))
            }
            found.push(FileInfo.joinPaths(product.buildDirectory, "libvpvl2.so*"))
            found.push(FileInfo.joinPaths(Qt.core.libPath, "libicu*.so.*"))
            return found
        }
        qbs.install: true
        qbs.installDir: "lib"
    }
    Group {
        condition: qbs.toolchain.contains("msvc")
        name: "Application Depending Libraries for MSVC"
        files: {
            var found = vpvm.findLibraries(commonLibraries.concat([ "OpenAL32", "AntTweakBar" ]),
                                           cpp.libraryPaths.concat([ "../openal-soft-src/" + libraryInstallDirectory + "/bin" ]),
                                           ".dll")
            for (var i in requiredSubmodules) {
                var requiredSubmodule = requiredSubmodules[i]
                var name = requiredSubmodule.toUpperCase().charAt(0) + requiredSubmodule.substring(1)
                found.push(FileInfo.joinPaths(Qt.core.binPath, "Qt5" + name + debugLibrarySuffix + ".dll"))
            }
            found.push(FileInfo.joinPaths(product.buildDirectory, "libvpvl2.so*"))
            var thirdPartyLibraries = [ "d3dcompiler_46", "icu*" + debugLibrarySuffix, "libEGL" + debugLibrarySuffix, "libGLESv2" + debugLibrarySuffix ]
            for (var i in thirdPartyLibraries) {
                found.push(FileInfo.joinPaths(Qt.core.binPath, thirdPartyLibraries[i] + ".dll"))
            }
            return found
        }
        qbs.install: true
    }
    Group {
        name: "QtQuick QML Resources"
        condition: !qbs.targetOS.contains("osx")
        files: [ Qt.core.binPath + "/../qml/QtQuick", Qt.core.binPath + "/../qml/QtQuick.2" ]
        qbs.install: qbs.buildVariant === "release"
        qbs.installDir: "qml"
    }
    Group {
        name: "Qt plugins"
        condition: !qbs.targetOS.contains("osx")
        files: {
            var plugins = [
                        "accessible",
                        "bearer",
                        "iconengines",
                        "imageformats",
                        "mediaservice",
                        "platforms",
                        "position",
                        "printsupport",
                        "sensorgestures",
                        "sensors"
                    ]
            if (!qbs.targetOS.contains("windows")) {
                plugins.push("audio")
                if (!qbs.targetOS.contains("osx")) {
                    plugins.push("generic", "platforminputcontexts", "platformthemes")
                }
            }
            return plugins.map(function(item){ return FileInfo.joinPaths(Qt.core.pluginPath, item) })
        }
        excludeFiles: [ "**/q*d.dll", "**/*.pdb" ]
        qbs.install: qbs.buildVariant === "release"
        qbs.installDir: "plugins"
    }
    Depends { name: "cpp" }
    Depends { name: "gizmo" }
    Depends { name: "vpvl2" }
    Depends {
        name: "Qt"
        submodules: requiredSubmodules
    }
}
