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

StaticLibrary {
    id: VPAPI
    readonly property string libraryBuildDirectory: "build-" + qbs.buildVariant.toLowerCase()
    readonly property string libraryInstallDirectory: libraryBuildDirectory + "/install-root"
    name: "VPAPI"
    files: [
        "src/*.cc",
        "include/*.h"
    ]
    cpp.includePaths: [
        buildDirectory,
        "include",
        "../libvpvl2/include",
        "../bullet-src/" + libraryInstallDirectory + "/include/bullet",
        "../tbb-src/include",
        "../glm-src"
    ]
    cpp.defines: [ "VPVL2_ENABLE_QT", "TW_STATIC", "TW_NO_LIB_PRAGMA" ]
    Properties {
        condition: qbs.targetOS.contains("osx")
        cpp.minimumOsxVersion: "10.6"
    }
    Properties {
        condition: qbs.toolchain.contains("msvc")
        consoleApplication: false
        cpp.cxxFlags: [ "/wd4068", "/wd4355", "/wd4819" ]
    }
    Depends { name: "cpp" }
    Depends { name: "vpvl2" }
    Depends { name: "Qt"; submodules: [ "core", "gui", "qml", "quick" ] }
}
