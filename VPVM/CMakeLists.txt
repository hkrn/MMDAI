cmake_minimum_required(VERSION 2.8)

# project configuration
project(VPVM)

set(VPVL2_ENABLE_EXTENSIONS_ARCHIVE ON)
set(VPVL2_ENABLE_EXTENSIONS_PROJECT ON)
set(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT ON)
set(VPVL2_ENABLE_EXTENSIONS_STRING ON)
set(VPVL2_ENABLE_EXTENSIONS_WORLD ON)
set(VPVL2_ENABLE_NVIDIA_CG ON)
set(VPVL2_ENABLE_OPENCL ON)
set(VPVL2_LINK_ASSIMP ON)
set(VPVL2_LINK_GLEW ON)
set(VPVL2_LINK_INTEL_TBB ON)
set(VPVL2_LINK_VPVL ON)

find_package(Qt4 4.8 REQUIRED QtCore QtGui QtOpenGL)
include(${QT_USE_FILE})
qt4_add_resources(vpvm_resource_src "${CMAKE_CURRENT_SOURCE_DIR}/resources/VPVM.qrc")

# include external cmake scripts
set(VPVL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../libvpvl2")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${VPVL2_PATH}/cmake")
include("${VPVL2_PATH}/cmake/vpvl2.cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

file(GLOB vpvm_sources_core "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cc")
source_group("VPVM (MMDAI2) Core Classes" FILES ${vpvm_sources_core})
file(GLOB vpvm_sources_dialogs "${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/*.cc")
source_group("VPVM (MMDAI2) Dialog Classes" FILES ${vpvm_sources_dialogs})
file(GLOB vpvm_sources_models "${CMAKE_CURRENT_SOURCE_DIR}/src/models/*.cc")
source_group("VPVM (MMDAI2) Models Classes" FILES ${vpvm_sources_models})
file(GLOB vpvm_sources_video "${CMAKE_CURRENT_SOURCE_DIR}/src/video/*.cc")
source_group("VPVM (MMDAI2) Video Classes" FILES ${vpvm_sources_video})
file(GLOB vpvm_sources_widgets "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/*.cc")
source_group("VPVM (MMDAI2) Widgets Classes" FILES ${vpvm_sources_widgets})
file(GLOB vpvm_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
source_group("VPVM (MMDAI2) Headers" FILES ${vpvm_headers})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(MMDAI2 MACOSX_BUNDLE WIN32
	          ${vpvm_sources_core} ${vpvm_sources_dialogs}
              ${vpvm_sources_models} ${vpvm_sources_video}
              ${vpvm_sources_widgets} ${vpvm_resource_src}
			  ${vpvm_headers})
set_target_properties(MMDAI2 PROPERTIES AUTOMOC ON)
target_link_libraries(MMDAI2 ${QT_LIBRARIES})

# libav
get_install_directory(LIBAV_INSTALL_DIRECTORY "libav-src")
find_library(AVCODEC_LIBRARY avcodec PATHS "${LIBAV_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
find_library(AVFORMAT_LIBRARY avformat PATHS "${LIBAV_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
find_library(AVUTIL_LIBRARY avutil PATHS "${LIBAV_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
find_library(SWSCALE_LIBRARY swscale PATHS "${LIBAV_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
target_link_libraries(MMDAI2 ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})
include_directories("${LIBAV_INSTALL_DIRECTORY}/include")

# OpenAL
get_install_directory(ALSOFT_INSTALL_DIRECTORY "openal-soft-src")
find_library(ALSOFT_LIBRARY openal PATHS "${ALSOFT_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
target_link_libraries(MMDAI2 ${ALSOFT_LIBRARY})
include_directories("${ALSOFT_INSTALL_DIRECTORY}/include")

# ALURE
get_install_directory(ALURE_INSTALL_DIRECTORY "alure-src")
find_library(ALURE_LIBRARY alure PATHS "${ALURE_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
target_link_libraries(MMDAI2 ${ALURE_LIBRARY})
include_directories("${ALURE_INSTALL_DIRECTORY}/include")

vpvl2_find_all()
vpvl2_link_all(MMDAI2)

# libvpvl2
get_build_directory(VPVL2_BUILD_DIRECTORY VPVL2_PATH OFF)
find_library(VPVL2_LIBRARY vpvl2 PATHS "${VPVL2_BUILD_DIRECTORY}/lib" NO_DEFAULT_PATH)
find_library(VPVL2QTCOMMON_LIBRARY vpvl2qtcommon PATHS "${VPVL2_BUILD_DIRECTORY}/lib" NO_DEFAULT_PATH)
target_link_libraries(MMDAI2 ${VPVL2_LIBRARY} ${VPVL2QTCOMMON_LIBRARY})
if(APPLE)
  find_library(COCOA_LIBRARY Cocoa)
  target_link_libraries(MMDAI2 ${COCOA_LIBRARY})
endif()
include_directories("${VPVL2_PATH}/include" "${VPVL2_BUILD_DIRECTORY}/include")
