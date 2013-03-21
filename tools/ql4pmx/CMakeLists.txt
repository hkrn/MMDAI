#
# === mesa (9.0.3) ===
#
# scons build=release machine=x86 llvm=no
# scons build=release machine=x86_64 llvm=yes
# mkdir build/release
# cd build/release
# lipo -create ../darwin-x86/mesa/libmesa.a ../darwin-x86/mesa/libmesa.a -output libmesa.a
# lipo -create ../darwin-x86/mesa/drivers/osmesa/libosmesa.dylib ../darwin-x86_64/mesa/drivers/osmesa/libosmesa.dylib -output libosmesa.dylib
# install_name_tool -id `pwd`/libosmesa.dylib libosmesa.dylib
#
# === libvpvl2 ===
#
# export BUILD_TYPE='Release'
# export INSTALL_ROOT=`pwd`/install-root
# export MESA_PATH=`pwd`"/../../mesa-src"
# export MESA3D_BINARY_DIR="$MESA_PATH/build/release"
# export MESA3D_INCLUDE_DIR="$MESA_PATH"
#
# CC=clang CXX=clang++ cmake -G Ninja -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_ROOT \
#     -DVPVL2_BUILD_QT_RENDERER:BOOL=OFF \
#     -DVPVL2_ENABLE_GLES2:BOOL=OFF \
#     -DVPVL2_ENABLE_NVIDIA_CG:BOOL=OFF \
#     -DVPVL2_ENABLE_OPENCL:BOOL=OFF \
#     -DVPVL2_ENABLE_OPENMP:BOOL=OFF \
#     -DVPVL2_ENABLE_EXTENSIONS_ARCHIVE:BOOL=OFF \
#     -DVPVL2_ENABLE_EXTENSIONS_PROJECT:BOOL=OFF \
#     -DVPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT:BOOL=ON \
#     -DVPVL2_ENABLE_EXTENSIONS_STRING:BOOL=ON \
#     -DVPVL2_ENABLE_EXTENSIONS_WORLD:BOOL=ON \
#     -DVPVL2_LINK_ASSIMP:BOOL=ON \
#     -DVPVL2_LINK_EGL:BOOL=OFF \
#     -DVPVL2_LINK_GLEW:BOOL=OFF \
#     -DVPVL2_LINK_INTEL_TBB:BOOL=OFF \
#     -DVPVL2_LINK_NVTT:BOOL=OFF \
#     -DVPVL2_LINK_QT:BOOL=OFF \
#     -DVPVL2_LINK_SDL1:BOOL=OFF \
#     -DVPVL2_LINK_SDL2:BOOL=OFF \
#     -DVPVL2_LINK_SFML:BOOL=OFF \
#     -DBUILD_SHARED_LIBS:BOOL=OFF \
#     -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE \
#     -DCMAKE_INSTALL_NAME_DIR:PATH="$INSTALL_ROOT/lib" \
#     -DCMAKE_OSX_ARCHITECTURES:INTERNAL="i386;x86_64" \
#     -DLIBRARY_OUTPUT_PATH:STRING="lib" \
#     -DVPVL2_ENABLE_OSMESA:BOOL=ON ..cmake_minimum_required(VERSION 2.8)
#
cmake_minimum_required(VERSION 2.8)

function(ql4pmx_link_libraries target)
  target_link_libraries(${target} ${VPVL2_LIBRARY} ${MESA3D_OSMESA_LIBRARY} ${MESA3D_MESA_LIBRARY})
  vpvl2_link_vpvl(${target})
  vpvl2_link_icu(${target})
  vpvl2_link_assimp(${target})
  vpvl2_link_bullet(${target})
  target_link_libraries(${target} ${FOUNDATION_FRAMEWORK} ${COCOA_FRAMEWORK} ${QUICKLOOK_FRAMEWORK})
endfunction()

set(CMAKE_CXX_CFLAGS "${CMAKE_CXX_CFLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
set(VPVL2_BASE_DIR_INTERNAL "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(VPVL2_CMAKE_DIR "${VPVL2_BASE_DIR_INTERNAL}/libvpvl2/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${VPVL2_CMAKE_DIR})
include("${VPVL2_CMAKE_DIR}/vpvl2.cmake")
set(VPVL2_BASE_DIR ${VPVL2_BASE_DIR_INTERNAL})

set(VPVL2_LINK_VPVL ON)
set(VPVL2_LINK_ASSIMP ON)
set(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT ON)
set(VPVL2_ENABLE_EXTENSIONS_STRING ON)

project(ql4pmx)
set(QL4PMX_BUNDLE_NAME "ql4pmx")
set(QL4PMX_MAJOR_VERSION 0)
set(QL4PMX_COMPAT_VERSION 1)
set(QL4PMX_MINOR_VERSION 0)
set(QL4PMX_VERSION "${QL4PMX_MAJOR_VERSION}.${QL4PMX_COMPAT_VERSION}.${QL4PMX_MINOR_VERSION}")

if(NOT CMAKE_BUILD_TYPE)
 set(CMAKE_BUILD_TYPE "Release")
endif()

get_source_directory(VPVL2_SOURCE_DIR "libvpvl2")
find_path(VPVL2_INCLUDE_DIR NAMES vpvl2/vpvl2.h PATH_SUFFIXES "build-${CMAKE_BUILD_TYPE}-ql4pmx/install-root/include" PATHS ${VPVL2_SOURCE_DIR} NO_DEFAULT_PATH)
find_library(VPVL2_LIBRARY vpvl2 PATH_SUFFIXES "build-${CMAKE_BUILD_TYPE}-ql4pmx/install-root/lib" PATHS ${VPVL2_SOURCE_DIR} NO_DEFAULT_PATH)

get_source_directory(MESA3D_SOURCE_DIR "mesa-src")
find_library(MESA3D_MESA_LIBRARY mesa PATH_SUFFIXES "release" PATHS "${MESA3D_SOURCE_DIR}/build" NO_DEFAULT_PATH)
find_library(MESA3D_OSMESA_LIBRARY osmesa PATH_SUFFIXES "release" "darwin-x86/mesa/drivers/osmesa" PATHS "${MESA3D_SOURCE_DIR}/build" NO_DEFAULT_PATH)
find_path(MESA3D_INCLUDE_DIR NAMES GL/osmesa.h PATH_SUFFIXES include PATHS ${MESA3D_SOURCE_DIR} NO_DEFAULT_PATH)

find_library(FOUNDATION_FRAMEWORK Foundation)
find_library(COCOA_FRAMEWORK Cocoa)
find_library(QUICKLOOK_FRAMEWORK QuickLook)
vpvl2_find_bullet()
vpvl2_find_assimp()
vpvl2_find_gli()
vpvl2_find_glm()
vpvl2_find_icu()
vpvl2_find_vpvl()

aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/src/common" ql4pmx_common)
aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/src/ql4pmx" ql4pmx_sources)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" ${VPVL2_INCLUDE_DIR} ${MESA3D_INCLUDE_DIR})
file(GLOB "${CMAKE_CURRENT_SOURCE_DIR}/include/*" ql4pmx_headers)

set(target "ql4pmx")
add_library(${target} MODULE ${ql4pmx_sources} ${ql4pmx_common} ${ql4pmx_headers})
set_target_properties(${target} PROPERTIES MACOSX_BUNDLE TRUE
                                BUNDLE TRUE
                                BUNDLE_EXTENSION "qlgenerator"
                                MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/res/Info.plist"
                                MACOSX_BUNDLE_BUNDLE_NAME "ql4pmx"
                                MACOSX_BUNDLE_SHORT_VERSION_STRING ${QL4PMX_VERSION}
                                MACOSX_BUNDLE_VERSION ${QL4PMX_VERSION})
ql4pmx_link_libraries(${target})

aux_source_directory("${CMAKE_CURRENT_SOURCE_DIR}/src/ql4test" ql4test_sources)
add_executable(ql4test ${ql4test_sources} ${ql4pmx_common} ${ql4pmx_headers})
ql4pmx_link_libraries(ql4test)

# cpack configuration
set(CPACK_PACKAGE_VERSION_MAJOR ${QL4PMX_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${QL4PMX_COMPAT_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${QL4PMX_MINOR_VERSION})
set(CPACK_PACKAGE_NAME ${QL4PMX_BUNDLE_NAME})
set(CPACK_PACKAGE_VENDOR "MMDAI2")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${QL4PMX_BUNDLE_NAME} is an QuickLook plugin to generate thumbnail or preview a model (PMD/PMX)")
include(CPack)
