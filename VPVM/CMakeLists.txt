cmake_minimum_required(VERSION 2.8)

# project configuration
project(VPVM)
set(VPVM_EXECUTABLE MMDAI2)
set(VPVM_VERSION_MAJOR "0")
set(VPVM_VERSION_MINOR "28")
set(VPVM_VERSION_PATCH "2")
set(VPVM_APPLICATION_NAME "${VPVM_EXECUTABLE}")
set(VPVM_APPLICATION_VENDOR "MMDAI")
set(VPVM_APPLICATION_DOMAIN "mmdai.github.com")
set(VPVM_APPLICATION_DOMAIN_REVERSED "com.github.mmdai")

option(VPVM_ENABLE_LIBAV "Enable link against libav to encode a video" ON)

# force below settings on for vpvl2.cmake
set(VPVL2_ENABLE_EXTENSIONS_ARCHIVE ON)
set(VPVL2_ENABLE_EXTENSIONS_PROJECT ON)
set(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT ON)
set(VPVL2_ENABLE_EXTENSIONS_STRING ON)
set(VPVL2_ENABLE_EXTENSIONS_WORLD ON)
set(VPVL2_ENABLE_NVIDIA_CG ON)
if(APPLE)
  set(VPVL2_ENABLE_OPENCL ON)
endif()
set(VPVL2_LINK_ASSIMP ON)
set(VPVL2_LINK_GLEW ON)
set(VPVL2_LINK_INTEL_TBB ON)
set(VPVL2_LINK_VPVL ON)
set(VPVL2_LINK_GLOG ON)

set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}:$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

# find Qt and generate resources with rcc
find_package(Qt4 4.8 REQUIRED QtCore QtGui QtOpenGL)
include(${QT_USE_FILE})

# include external cmake scripts
set(VPVL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../libvpvl2")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${VPVL2_PATH}/cmake")
include("${VPVL2_PATH}/cmake/vpvl2.cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# find all sources
file(GLOB vpvm_sources_core "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cc")
source_group("VPVM (MMDAI2) Core Classes" FILES ${vpvm_sources_core})
file(GLOB vpvm_sources_dialogs "${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/*.cc")
source_group("VPVM (MMDAI2) Dialog Classes" FILES ${vpvm_sources_dialogs})
file(GLOB vpvm_sources_models "${CMAKE_CURRENT_SOURCE_DIR}/src/models/*.cc")
source_group("VPVM (MMDAI2) Models Classes" FILES ${vpvm_sources_models})
file(GLOB vpvm_sources_widgets "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/*.cc")
source_group("VPVM (MMDAI2) Widgets Classes" FILES ${vpvm_sources_widgets})
file(GLOB vpvm_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
source_group("VPVM (MMDAI2) Headers" FILES ${vpvm_headers})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" ${CMAKE_CURRENT_BINARY_DIR})

if(VPVM_ENABLE_LIBAV)
  file(GLOB vpvm_sources_video "${CMAKE_CURRENT_SOURCE_DIR}/src/video/*.cc")
  add_definitions(-DVPVM_ENABLE_LIBAV)
else()
  file(GLOB vpvm_sources_video "${CMAKE_CURRENT_SOURCE_DIR}/src/video/AVFactory.cc")
  list(REMOVE_ITEM vpvm_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/AVCommon.h")
  list(REMOVE_ITEM vpvm_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/AudioDecoder.h")
  list(REMOVE_ITEM vpvm_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/VideoEncoder.h")
endif()
source_group("VPVM (MMDAI2) Video Classes" FILES ${vpvm_sources_video})

# set application icon
if(APPLE)
  set(MACOSX_BUNDLE_ICON_FILE app.icns)
  set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  set(vpvm_resource_src ${vpvm_resource_src} "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app.icns")
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/resources/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

# generate resources
qt4_add_resources(vpvm_resource_src "${CMAKE_CURRENT_SOURCE_DIR}/resources/VPVM.qrc")
qt4_create_translation(vpvm_qm_files
                       ${vpvm_sources_core} ${vpvm_sources_dialogs}
                       ${vpvm_sources_models} ${vpvm_sources_video}
                       ${vpvm_sources_widgets} ${vpvm_headers}
                       "${CMAKE_CURRENT_SOURCE_DIR}/resources/translations/VPVM_ja.ts")
add_custom_target(vpvm_translation_target SOURCES ${vpvm_qm_files})

# generate executable
add_executable(${VPVM_EXECUTABLE} MACOSX_BUNDLE WIN32
                    ${vpvm_sources_core} ${vpvm_sources_dialogs}
              ${vpvm_sources_models} ${vpvm_sources_video}
              ${vpvm_sources_widgets} ${vpvm_resource_src}
                                ${vpvm_headers})
add_dependencies(${VPVM_EXECUTABLE} vpvm_translation_target)
set_target_properties(${VPVM_EXECUTABLE} PROPERTIES AUTOMOC ON)
target_link_libraries(${VPVM_EXECUTABLE} ${QT_LIBRARIES} ${QT_QTMAIN_LIBRARY})

if(APPLE)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
  set_target_properties(${VPVM_EXECUTABLE} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
endif()

# libav
if(VPVM_ENABLE_LIBAV)
  get_install_directory(LIBAV_INSTALL_DIRECTORY "libav-src")
  find_library(AVCODEC_LIBRARY avcodec PATHS "${LIBAV_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
  find_library(AVFORMAT_LIBRARY avformat PATHS "${LIBAV_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
  find_library(AVUTIL_LIBRARY avutil PATHS "${LIBAV_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
  find_library(SWSCALE_LIBRARY swscale PATHS "${LIBAV_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
  target_link_libraries(${VPVM_EXECUTABLE} ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})
  include_directories("${LIBAV_INSTALL_DIRECTORY}/include")
endif()

# OpenAL
get_install_directory(ALSOFT_INSTALL_DIRECTORY "openal-soft-src")
find_library(ALSOFT_LIBRARY openal PATHS "${ALSOFT_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
target_link_libraries(${VPVM_EXECUTABLE} ${ALSOFT_LIBRARY})
include_directories("${ALSOFT_INSTALL_DIRECTORY}/include")

# ALURE
get_install_directory(ALURE_INSTALL_DIRECTORY "alure-src")
find_library(ALURE_LIBRARY alure PATHS "${ALURE_INSTALL_DIRECTORY}/lib" NO_DEFAULT_PATH)
target_link_libraries(${VPVM_EXECUTABLE} ${ALURE_LIBRARY})
include_directories("${ALURE_INSTALL_DIRECTORY}/include")

# libvpvl2
get_build_directory(VPVL2_BUILD_DIRECTORY VPVL2_PATH OFF)
find_library(VPVL2_LIBRARY vpvl2 PATHS "${VPVL2_BUILD_DIRECTORY}/lib" NO_DEFAULT_PATH)
find_library(VPVL2QTCOMMON_LIBRARY vpvl2qtcommon PATHS "${VPVL2_BUILD_DIRECTORY}/lib" NO_DEFAULT_PATH)
target_link_libraries(${VPVM_EXECUTABLE} ${VPVL2_LIBRARY} ${VPVL2QTCOMMON_LIBRARY})
if(APPLE)
  find_library(COCOA_LIBRARY Cocoa)
  target_link_libraries(${VPVM_EXECUTABLE} ${COCOA_LIBRARY})
endif()
include_directories("${VPVL2_PATH}/include" "${VPVL2_BUILD_DIRECTORY}/include")

vpvl2_find_all()
vpvl2_link_all(${VPVM_EXECUTABLE})
vpvl2_link_alsoft(${VPVM_EXECUTABLE})
vpvl2_link_alure(${VPVM_EXECUTABLE})
vpvl2_set_warnings()

# configure installation
set(VPVM_PLUGIN_DIR "plugins")
set(VPVM_LOCALES_DIR "locales")
set(VPVM_RESOURCES_DIR "resources")
set(VPVM_LIBRARY_DIRS ${QT_LIBRARY_DIRS})
if(APPLE)
  set(VPVM_PLUGIN_DIR "${VPVM_EXECUTABLE}.app/Contents/PlugIns")
  set(VPVM_LOCALES_DIR "${VPVM_EXECUTABLE}.app/Contents/Resources")
  set(VPVM_RESOURCES_DIR "${VPVM_EXECUTABLE}.app/Contents/Resources")
  set(VPVM_BINARY_DIR "${CMAKE_INSTALL_PREFIX}/${VPVM_EXECUTABLE}.app")
elseif(WIN32)
  set(VPVM_BINARY_DIR "${CMAKE_INSTALL_PREFIX}/${VPVM_EXECUTABLE}.exe")
else()
  set(VPVM_BINARY_DIR "${CMAKE_INSTALL_PREFIX}/${VPVM_EXECUTABLE}")
endif()

install(TARGETS ${VPVM_EXECUTABLE} BUNDLE DESTINATION . COMPONENT Runtime
                                   RUNTIME DESTINATION . COMPONENT Runtime)
file(GLOB vpvm_translation_files ${vpvm_qm_files} "${QT_TRANSLATIONS_DIR}/qt_*.qm")
install(FILES ${vpvm_translation_files} DESTINATION "${VPVM_RESOURCES_DIR}")
install(FILES ${vpvm_qm_files} DESTINATION "${VPVM_RESOURCES_DIR}")
install(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${VPVM_PLUGIN_DIR})

install(CODE "
  file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${VPVM_RESOURCES_DIR}/qt.conf\" \"\")
  file(GLOB_RECURSE QTPLUGINS \"\${CMAKE_INSTALL_PREFIX}/${VPVM_PLUGIN_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
  #file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/lib\")
  list(APPEND DIRS_TO_FIND_LIBRARY \"${LIBAV_INSTALL_DIRECTORY}/lib\")
  list(APPEND DIRS_TO_FIND_LIBRARY \"${ALSOFT_INSTALL_DIRECTORY}/lib\")
  list(APPEND DIRS_TO_FIND_LIBRARY \"${ALURE_INSTALL_DIRECTORY}/lib\")
  include(BundleUtilities)
  fixup_bundle(\"${VPVM_BINARY_DIR}\" \"\${QTPLUGINS}\" \"\${DIRS_TO_FIND_LIBRARY}\")
  get_bundle_keys(\"${VPVM_BINARY_DIR}\" \"\" \"\${DIRS_TO_FIND_LIBRARY}\" keys)
  foreach(key \${keys})
    list(APPEND CMAKE_INSTALL_MANIFEST_FILES \"\${\${key}_RESOLVED_EMBEDDED_ITEM}\")
  endforeach()
  " COMPONENT Runtime)

# cpack configuration
set(CPACK_PACKAGE_VERSION_MAJOR ${VPVM_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VPVM_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VPVM_VERSION_PATCH})
set(CPACK_PACKAGE_NAME ${VPVM_APPLICATION_NAME})
set(CPACK_PACKAGE_VENDOR ${VPVM_APPLICATION_VENDOR})
set(CPACK_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)
set(CPACK_STRIP_FILES ON)
include(CPack)
