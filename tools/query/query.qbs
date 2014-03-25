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
import qbs.FileInfo

CppApplication {
    id: MikuMikuQuery
    readonly property string libraryBuildDirectory: "build-" + qbs.buildVariant.toLowerCase()
    readonly property string libraryInstallDirectory: libraryBuildDirectory + "/install-root"
    readonly property string debugLibrarySuffix: qbs.enableDebugCode ? "d" : ""
    readonly property string assimpLibrarySuffix: qbs.toolchain.contains("msvc") ? "" : debugLibrarySuffix.toUpperCase()
    readonly property string nvFXLibrarySuffix: (cpp.architecture === "x86_64" ? "64" : "") + debugLibrarySuffix.toUpperCase()
    readonly property var commonLibraries: [
        "assimp" + assimpLibrarySuffix,
        "FxParser" + nvFXLibrarySuffix,
        "FxLibGL" + nvFXLibrarySuffix,
        "FxLib" + nvFXLibrarySuffix,
        "BulletSoftBody",
        "BulletDynamics",
        "BulletCollision",
        "LinearMath"
    ]
    name: "MikuMikuQuery"
    cpp.includePaths: [ buildDirectory ].concat([
        "include",
        "../../bullet-src/" + libraryInstallDirectory + "/include/bullet",
        "../../libvpvl2/include",
        "../../glm-src",
        "../../alure-src/include",
        "../../tbb-src/include",
    ].map(function(x){ return FileInfo.joinPaths(sourceDirectory, x) }))
    cpp.libraryPaths: [ FileInfo.joinPaths(sourceDirectory, "../../tbb-src/lib") ].concat([
        "../../bullet-src",
        "../../assimp-src",
        "../../nvFX-src",
        "../../alure-src",
        "../../openal-soft-src",
        "../../icu4c-src",
        "../../zlib-src"
    ].map(function(x){ return FileInfo.joinPaths(sourceDirectory, x, libraryInstallDirectory, "lib") }))
    cpp.dynamicLibraries: commonLibraries.concat([ "tbb", "z" ])
    files: [
        "src/*.cc",
        "include/*.h",
        "resources/*.qrc"
    ].map(function(path){ return FileInfo.joinPaths(sourceDirectory, path) })
    Depends { name: "vpvl2" }
    Depends { name: "Qt"; submodules: [ "core", "sql" ] }
}
