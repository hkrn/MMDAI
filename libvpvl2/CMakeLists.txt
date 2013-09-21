cmake_minimum_required(VERSION 2.8.9)

# declare project configuration
project(libvpvl2)

# library version setting
set(VPVL2_VERSION_MAJOR 0)
set(VPVL2_VERSION_COMPAT 12)
set(VPVL2_VERSION_MINOR 0)
set(VPVL2_PROJECT_NAME "vpvl2")

# include external cmake scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/vpvl2.cmake")

# declare options
option(BUILD_SHARED_LIBS "Build Shared Libraries (default is OFF)" OFF)
option(VPVL2_BUILD_QT_RENDERER "Build a renderer program using Qt 4.8 (enabling VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT is required, default is OFF)" OFF)
option(VPVL2_COORDINATE_OPENGL "Use OpenGL coordinate system (default is ON)" ON)

option(VPVL2_ENABLE_NVIDIA_CG "Include OpenGL renderer class using NVIDIA Cg (default is OFF)" OFF)
option(VPVL2_ENABLE_OPENCL "Enable OpenCL acceleration (default is OFF)" OFF)
option(VPVL2_ENABLE_OPENMP "Enable OpenMP acceleration (default is OFF)" OFF)
option(VPVL2_ENABLE_EXTENSIONS_ARCHIVE "Include classes of loading zip files (default is OFF)" OFF)
option(VPVL2_ENABLE_EXTENSIONS_EFFECT "Include classes of loading effect files (default is OFF)" OFF)
option(VPVL2_ENABLE_EXTENSIONS_PROJECT "Include classes of loading and saving project files (default is OFF)" OFF)
option(VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT "Include classes of base implementation of IApplicationContext (default is OFF)" OFF)
option(VPVL2_ENABLE_EXTENSIONS_STRING "Include classes of implementation of IEncoding/IString based on ICU (default is OFF)" OFF)
option(VPVL2_ENABLE_EXTENSIONS_WORLD "Include classes of physics world for model based on Bullet Physics (default is OFF)" OFF)
option(VPVL2_ENABLE_GLES2 "Build a library for GLES2 compliant (enabling VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT is required default is OFF)" OFF)
option(VPVL2_ENABLE_OSMESA "Build a library using Offscreen Mesa3D software rasterizer instead of default OpenGL runtime (enabling VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT is required, default is OFF)" OFF)
option(VPVL2_ENABLE_TEST "Build a unit test runtime (enabling VPVL2_LINK_QT is required, default is OFF)" OFF)
option(VPVL2_ENABLE_COMPILER_TLS "Build library with compiler TLS (default is OFF)" OFF)
option(VPVL2_ENABLE_LAZY_LINK "Doesn't link immediately after compiling libvpvl2 for LLVM bitcode such as Emscripten (default is OFF)" OFF)

option(VPVL2_LINK_SDL2 "Link against SDL 2.0 (enabling VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT is required, default is OFF)" OFF)
option(VPVL2_LINK_ASSIMP3 "Link against Open Asset Import Library 3.x (default is OFF)" OFF)
option(VPVL2_LINK_ASSIMP "Link against Open Asset Import Library 2.x (default is OFF)" OFF)
option(VPVL2_LINK_VPVL "Link against libvpvl (default is OFF)" OFF)
option(VPVL2_LINK_INTEL_TBB "Link against Intel Threading Building Blocks (default is OFF)" OFF)
option(VPVL2_LINK_SFML "Link against SFML 2.0 (enabling VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT is required, default is OFF)" OFF)
option(VPVL2_LINK_EGL "Link against EGL (enabling VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT is required, default is OFF)" OFF)
option(VPVL2_LINK_QT "Link against Qt 4.8 or higher (enabling VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT required, default is OFF)" OFF)
option(VPVL2_LINK_GLOG "Link against glog to enable logging globally (default is OFF)" OFF)
option(VPVL2_LINK_REGAL "Link against Regal (default is OFF)" OFF)
option(VPVL2_LINK_GLFW "Link against GLFW 3.x (default is OFF)" OFF)
option(VPVL2_LINK_NVFX "Link against nvFX (default is OFF)" OFF)
option(VPVL2_LINK_ATB "Link against AntTweakBar (default is OFF)" OFF)
option(VPVL2_LINK_FREEIMAGE "Link against FreeImage (default is OFF)" OFF)
option(VPVL2_LINK_ALLEGRO "Link against Allegro 5.x (enabling VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT is required, default is OFF)" OFF)

option(VPVL2_PLATFORM_EMSCRIPTEN "Build a library for Emscripten (VPVL2_ENABLE_GLES2 is required, default is OFF)" OFF)
option(VPVL2_PLATFORM_RASPBERRY_PI "Build a library for Raspberry Pi (VPVL2_ENABLE_GLES2 is required, default is OFF)" OFF)

# set library version
set(VPVL2_VERSION "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.${VPVL2_VERSION_MINOR}")
set(VPVL2_VERSION_COMPATIBLE "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.0")

# detect OS and write it to config.h.in
vpvl2_detect_os()

# source configuration
file(GLOB vpvl2_sources_core "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cc")
file(GLOB vpvl2_sources_base "${CMAKE_CURRENT_SOURCE_DIR}/src/core/base/*.cc")
source_group("VPVL2 Core Classes" FILES ${vpvl2_sources_core} ${vpvl2_sources_base})
file(GLOB vpvl2_sources_internal "${CMAKE_CURRENT_SOURCE_DIR}/src/core/internal/*.cc")
file(GLOB vpvl2_headers_internal "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/internal/*.h")
source_group("VPVL2 Internal Classes" FILES ${vpvl2_sources_internal} ${vpvl2_headers_internal})
file(GLOB vpvl2_sources_asset "${CMAKE_CURRENT_SOURCE_DIR}/src/core/asset/*.cc")
file(GLOB vpvl2_headers_asset "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/asset/*.h")
source_group("VPVL2 Asset Classes" FILES ${vpvl2_sources_asset} ${vpvl2_headers_asset})
file(GLOB vpvl2_sources_vmd "${CMAKE_CURRENT_SOURCE_DIR}/src/core/vmd/*.cc")
file(GLOB vpvl2_headers_vmd "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/*.h")
source_group("VPVL2 VMD Classes" FILES ${vpvl2_sources_vmd} ${vpvl2_headers_vmd})
file(GLOB vpvl2_sources_mvd "${CMAKE_CURRENT_SOURCE_DIR}/src/core/mvd/*.cc")
file(GLOB vpvl2_headers_mvd "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/*.h")
source_group("VPVL2 MVD Classes" FILES ${vpvl2_sources_mvd} ${vpvl2_headers_mvd})
if(VPVL2_LINK_VPVL)
  file(GLOB vpvl2_sources_pmd "${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmd/*.cc")
  file(GLOB vpvl2_headers_pmd "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/*.h")
else()
  file(GLOB vpvl2_sources_pmd "${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmd2/*.cc")
  file(GLOB vpvl2_headers_pmd "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/*.h")
endif()
source_group("VPVL2 PMD Classes" FILES ${vpvl2_sources_pmd} ${vpvl2_headers_pmd})
file(GLOB vpvl2_sources_pmx "${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmx/*.cc")
file(GLOB vpvl2_headers_pmx "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/*.h")
source_group("VPVL2 PMX Classes" FILES ${vpvl2_sources_pmx} ${vpvl2_headers_pmx})

list(APPEND vpvl2_sources ${vpvl2_sources_core} ${vpvl2_sources_base} ${vpvl2_sources_internal}
                          ${vpvl2_sources_asset} ${vpvl2_sources_vmd} ${vpvl2_sources_mvd}
                          ${vpvl2_sources_pmd} ${vpvl2_sources_pmx})

# extensions configuration
if(VPVL2_ENABLE_EXTENSIONS_ARCHIVE)
  if(APPLE)
    add_definitions(-DUSE_FILE32API)
  endif()
  file(GLOB vpvl2_sources_archive "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/Archive.cc")
  set(MINIZIP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/minizip-1.1")
  file(GLOB vpvl2_sources_minizip "${MINIZIP_ROOT}/*.c")
  file(GLOB vpvl2_headers_minizip "${MINIZIP_ROOT}/*.h")
  include_directories(${MINIZIP_ROOT})
  source_group("minizip 1.1" FILES ${vpvl2_sources_minizip} ${vpvl2_headers_minizip})
  list(APPEND vpvl2_sources ${vpvl2_sources_minizip} ${vpvl2_headers_minizip})
  source_group("VPVL2 Archive Classes" FILES ${vpvl2_sources_archive})
  list(APPEND vpvl2_sources ${vpvl2_sources_archive})
endif()

# ApplicationContext extension
if(VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT)
  set(SOIL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/SOIL")
  file(GLOB vpvl2_sources_soil "${SOIL_ROOT}/*.c")
  file(GLOB vpvl2_headers_soil "${SOIL_ROOT}/*.h")
  include_directories(${SOIL_ROOT})
  source_group("Simple OpenGL Image Library" FILES ${vpvl2_sources_soil} ${vpvl2_headers_soil})
  list(APPEND vpvl2_sources ${vpvl2_sources_soil} ${vpvl2_headers_soil})
  file(GLOB vpvl2_sources_render_context "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/BaseApplicationContext.cc")
  file(GLOB vpvl2_headers_render_context "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/BaseApplicationContext.h")
  source_group("VPVL2 ApplicationContext Classes" FILES ${vpvl2_sources_render_context} ${vpvl2_headers_render_context})
  list(APPEND vpvl2_sources ${vpvl2_sources_render_context} ${vpvl2_headers_render_context})
endif()

# Project extension
if(VPVL2_ENABLE_EXTENSIONS_PROJECT)
  file(GLOB vpvl2_sources_project "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/XMLProject.cc")
  file(GLOB vpvl2_headers_project "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/XMLProject.h")
  source_group("VPVL2 Project Classes" FILES ${vpvl2_sources_project} ${vpvl2_headers_project})
  list(APPEND vpvl2_sources ${vpvl2_sources_project} ${vpvl2_headers_project})
endif()

# ICU4C based String extension
if(VPVL2_ENABLE_EXTENSIONS_STRING)
  file(GLOB vpvl2_sources_string "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/Encoding.cc"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/String*.cc")
  file(GLOB vpvl2_headers_string "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/icu/*.h")
  source_group("VPVL2 String Classes" FILES ${vpvl2_sources_string} ${vpvl2_headers_string})
  list(APPEND vpvl2_sources ${vpvl2_sources_string} ${vpvl2_headers_string})
endif()

# Bullet Physics World extension
if(VPVL2_ENABLE_EXTENSIONS_WORLD)
  file(GLOB vpvl2_sources_world "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/World.cc")
  file(GLOB vpvl2_headers_world "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/World.h")
  source_group("VPVL2 World Classes" FILES ${vpvl2_sources_world} ${vpvl2_headers_world})
  list(APPEND vpvl2_sources ${vpvl2_sources_world} ${vpvl2_headers_world})
endif()

# Effect extension
if(VPVL2_ENABLE_EXTENSIONS_EFFECT)
  file(GLOB vpvl2_sources_fx "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/*FX5.cc")
  file(GLOB vpvl2_headers_fx "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/fx/*")
  source_group("VPVL2 Effect Classes" FILES ${vpvl2_sources_fx} ${vpvl2_headers_fx})
  list(APPEND vpvl2_sources ${vpvl2_sources_fx} ${vpvl2_headers_fx})
endif()

# Bundled tinyxml2
if(VPVL2_ENABLE_EXTENSIONS_PROJECT OR VPVL2_ENABLE_NVIDIA_CG OR VPVL2_LINK_NVFX)
  set(TINYXML2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyxml2-1.0.11")
  file(GLOB vpvl2_sources_tinyxml2 "${TINYXML2_ROOT}/*.cpp")
  file(GLOB vpvl2_headers_tinyxml2 "${TINYXML2_ROOT}/*.h")
  include_directories(${TINYXML2_ROOT})
  source_group("TinyXML2 1.0.11" FILES ${vpvl2_sources_tinyxml2} ${vpvl2_headers_tinyxml2})
  list(APPEND vpvl2_sources ${vpvl2_sources_tinyxml2} ${vpvl2_headers_tinyxml2})
endif()

file(GLOB vpvl2_public_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/*.h")
file(GLOB_RECURSE vpvl2_public_headers_extensions "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/*.h")
list(APPEND vpvl2_internal_headers ${vpvl2_headers_internal} ${vpvl2_headers_asset} ${vpvl2_headers_vmd}
                                   ${vpvl2_headers_mvd} ${vpvl2_headers_pmd} ${vpvl2_headers_pmx})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
if(VPVL2_ENABLE_CUSTOM_RELEASE_CLANG AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_C_FLAGS_RELEASE "-O2 -flimit-debug-info -gline-tables-only" CACHE STRINGS "" FORCE)
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -flimit-debug-info -gline-tables-only" CACHE STRINGS "" FORCE)
  add_definitions(-DNDEBUG)
endif()

# dynamic library or static library
if(BUILD_SHARED_LIBS)
  set(VPVL2_LIB_TYPE SHARED)
else()
  set(VPVL2_LIB_TYPE STATIC)
endif()

# intercept to add source (OpenGL)
if(VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT)
  file(GLOB vpvl2_sources_engines_gl "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/gl2/*.cc")
  file(GLOB vpvl2_headers_engines_gl "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/gl2/*.h")
  source_group("VPVL2 OpenGL Render Engine" FILES ${vpvl2_sources_engines_gl} ${vpvl2_headers_engines_gl})
  list(APPEND vpvl2_internal_headers ${vpvl2_headers_engines_gl})
  list(APPEND vpvl2_sources ${vpvl2_sources_engines_gl})
  if(VPVL2_ENABLE_NVIDIA_CG OR VPVL2_LINK_NVFX)
    file(GLOB vpvl2_sources_engines_fx "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/fx/*.cc")
    file(GLOB vpvl2_headers_engines_fx "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/fx/*.h")
    source_group("VPVL2 Common Effect Engine" FILES ${vpvl2_sources_engines_fx} ${vpvl2_headers_engines_fx})
    list(APPEND vpvl2_internal_headers ${vpvl2_headers_engines_fx})
    list(APPEND vpvl2_sources ${vpvl2_sources_engines_fx})
    if(VPVL2_LINK_NVFX)
      file(GLOB vpvl2_sources_engines_nvfx "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/nvfx/*.cc")
      file(GLOB vpvl2_headers_engines_nvfx "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/nvfx/*.h")
      file(GLOB vpvl2_glew_engines_nvfx "${CMAKE_CURRENT_SOURCE_DIR}/vendor/GLEW-1.10.0/glew.c")
      add_definitions(-DGLEW_STATIC -DGLEW_NO_GLU)
      include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vendor/GLEW-1.10.0")
      source_group("VPVL2 OpenGL Render Engine with nvFX" FILES ${vpvl2_sources_engines_nvfx} ${vpvl2_headers_engines_nvfx})
      list(APPEND vpvl2_internal_headers ${vpvl2_headers_engines_nvfx})
      list(APPEND vpvl2_sources ${vpvl2_sources_engines_nvfx} ${vpvl2_glew_engines_nvfx})
    endif()
    if(VPVL2_ENABLE_NVIDIA_CG)
      file(GLOB vpvl2_sources_engines_cg "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/cg/*.cc")
      file(GLOB vpvl2_headers_engines_cg "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cg/*.h")
      source_group("VPVL2 OpenGL Render Engine with Cg" FILES ${vpvl2_sources_engines_cg} ${vpvl2_headers_engines_cg})
      list(APPEND vpvl2_internal_headers ${vpvl2_headers_engines_cg})
      list(APPEND vpvl2_sources ${vpvl2_sources_engines_cg})
    endif()
  endif()
  if(VPVL2_ENABLE_OPENCL)
    set(OPENCL_CXX_BINDINGS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cl-1.2")
    file(GLOB vpvl2_sources_accelerator_cl "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/cl/*.cc")
    file(GLOB vpvl2_headers_accelerator_cl "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cl/*.h"
                                           "${OPENCL_CXX_BINDINGS_ROOT}/CL/*.hpp")
    include_directories(${OPENCL_CXX_BINDINGS_ROOT})
    source_group("VPVL2 OpenCL Accelerator" FILES ${vpvl2_sources_accelerator_cl} ${vpvl2_headers_accelerator_cl})
    list(APPEND vpvl2_internal_headers ${vpvl2_headers_accelerator_cl})
    list(APPEND vpvl2_sources ${vpvl2_sources_accelerator_cl})
  endif()
  set(VPVL2_COORDINATE_OPENGL ON BOOL)
  set(VPVL2_ENABLE_OPENGL ON BOOL)
endif()

# Bundled glloadgen generated sources
if(VPVL2_ENABLE_OPENGL)
  set(GLLOADGEN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glloadgen")
  file(GLOB vpvl2_sources_glloadgen "${GLLOADGEN_ROOT}/*.c")
  file(GLOB vpvl2_headers_glloadgen "${GLLOADGEN_ROOT}/*.h")
  include_directories(${GLLOADGEN_ROOT})
  source_group("glLoadGenerator" FILES ${vpvl2_sources_glloadgen} ${vpvl2_headers_glloadgen})
  list(APPEND vpvl2_sources ${vpvl2_sources_glloadgen} ${vpvl2_headers_glloadgen})
endif()

# check compiler static tls
if(VPVL2_ENABLE_COMPILER_TLS)
  include(CheckCXXSourceCompiles)
  check_cxx_source_compiles("__thread int i; int main() { return 0; }" VPVL2_HAS_STATIC_TLS_GNU)
  if(NOT VPVL2_HAS_STATIC_TLS_GNU)
    check_cxx_source_compiles("__declspec(thread) int i; int main() { return 0; }" VPVL2_HAS_STATIC_TLS_MSVC)
  endif()
endif()

# generate configuration file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h")
set(vpvl2_public_headers ${vpvl2_public_headers} "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h")

# create libvpvl2 project
add_library(${VPVL2_PROJECT_NAME} ${VPVL2_LIB_TYPE} ${vpvl2_sources} ${vpvl2_public_headers} ${vpvl2_public_headers_extensions} ${vpvl2_internal_headers})
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")

# configure library properties
set_target_properties(${VPVL2_PROJECT_NAME} PROPERTIES VERSION ${VPVL2_VERSION} SOVERSION ${VPVL2_VERSION_COMPATIBLE})
set_target_properties(${VPVL2_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${VPVL2_PROJECT_NAME})
vpvl2_set_library_properties(${VPVL2_PROJECT_NAME} ${vpvl2_public_headers})
vpvl2_set_warnings()

# project include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
# this for vpvl2/config.h
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# find and link libraries to libvpvl2
vpvl2_find_all()
if(BUILD_SHARED_LIBS OR MSVC)
  vpvl2_link_all(${VPVL2_PROJECT_NAME})
endif()

# link against Qt
if(VPVL2_ENABLE_EXTENSIONS_APPLICATIONCONTEXT)
  vpvl2_add_qt_helpers()
  vpvl2_add_sdl_renderer()
  vpvl2_add_sfml_renderer()
  vpvl2_add_glfw_renderer()
  vpvl2_add_allegro_renderer()
  vpvl2_add_egl_renderer()
endif()

# generate pkg-config
if(NOT WIN32)
  set(PKGCONFIG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig" CACHE STRING "Base directory for pkgconfig files")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vpvl2.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc" DESTINATION "${PKGCONFIG_INSTALL_PREFIX}")
endif()

# install configuration
install(TARGETS ${VPVL2_PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
if(vpvl2qtcommon_sources_common)
  install(TARGETS vpvl2qtcommon DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
endif()
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/gl_vpvl2.h" DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2" DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2" DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
        PATTERN "*.h"
        PATTERN ".svn" EXCLUDE
        PATTERN "config.h.in" EXCLUDE
        PATTERN "internal" EXCLUDE
        PATTERN "asset" EXCLUDE
        PATTERN "vmd" EXCLUDE
        PATTERN "mvd" EXCLUDE
        PATTERN "pmd" EXCLUDE
        PATTERN "pmd2" EXCLUDE
        PATTERN "pmx" EXCLUDE
        PATTERN "cg" EXCLUDE
        PATTERN "cl" EXCLUDE
        PATTERN "fx" EXCLUDE
        PATTERN "nvfx" EXCLUDE
        PATTERN "gl2" EXCLUDE
        PATTERN "qt" EXCLUDE)

if(VPVL2_LINK_QT)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vpvl2")
endif()
