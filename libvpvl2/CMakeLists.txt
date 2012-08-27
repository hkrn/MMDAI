cmake_minimum_required(VERSION 2.6)

set(VPVL2_VERSION_MAJOR 0)
set(VPVL2_VERSION_COMPAT 5)
set(VPVL2_VERSION_MINOR 0)

# set library version
set(VPVL2_VERSION "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.${VPVL2_VERSION_MINOR}")
set(VPVL2_VERSION_COMPATIBLE "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.0")

# project configuration
project(libvpvl2)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/common vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/mvd vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmx vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/vmd vpvl2_sources)
set(vpvl2_public_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/Common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/Factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vpvl2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IBone.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IBoneKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/ICamera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/ICameraKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IEffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IEncoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/ILabel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/ILight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/ILightKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IMorph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IMorphKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IMotion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IRenderDelegate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IRenderEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IString.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/Scene.h
)
set(vpvl2_internal_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/internal/util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/AssetKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/AssetSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/BaseSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/BoneKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/BoneSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/CameraKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/CameraSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/EffectKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/EffectSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/LightKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/LightSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/ModelKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/ModelSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/MorphKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/MorphSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/Motion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/NameListSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/ProjectKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/ProjectSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/Bone.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/Joint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/Label.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/Material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/Model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/Morph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/RigidBody.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/Vertex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/BaseAnimation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/BaseKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/BoneAnimation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/BoneKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/CameraAnimation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/CameraKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/LightAnimation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/LightKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/MorphAnimation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/MorphKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/Motion.h
)

IF(NOT CMAKE_BUILD_TYPE)
 SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

# declaration of function
function(link_vpvl target)
  get_build_type(LIBVPVL_BUILD_TYPE)
  set(LIBVPVL_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libvpvl")
  find_path(LIBVPVL_INCLUDE_DIRS vpvl/vpvl.h PATHS "${LIBVPVL_LOCAL_DIR}/include")
  find_path(LIBVPVL_CONFIG_INCLUDE_DIRS vpvl/config.h PATHS "${LIBVPVL_LOCAL_DIR}/${LIBVPVL_BUILD_TYPE}/include")
  include_directories(${LIBVPVL_INCLUDE_DIRS} ${LIBVPVL_CONFIG_INCLUDE_DIRS})
  if(BUILD_SHARED_LIBS)
    if(LIBVPVL_BUILD_TYPE STREQUAL "debug")
      find_library(LIBVPVL_LIB vpvl_debug PATHS "${LIBVPVL_LOCAL_DIR}/${LIBVPVL_BUILD_TYPE}/lib")
    else()
      find_library(LIBVPVL_LIB vpvl PATHS "${LIBVPVL_LOCAL_DIR}/${LIBVPVL_BUILD_TYPE}/lib")
    endif()
    target_link_libraries(${target} ${LIBVPVL_LIB})
  endif()
endfunction()

# declarations of function
function(link_bullet target)
  get_build_type(BULLET_BUILD_TYPE)
  set(BULLET_LIBRARY_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bullet/${BULLET_BUILD_TYPE}/lib")
  # find from MMDAI/bullet or environment variables
  find_path(BULLET_INCLUDE_DIRS btBulletDynamicsCommon.h PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../bullet/src" $ENV{BULLET_INCLUDE_DIR})
  include_directories(${BULLET_INCLUDE_DIRS})
  if(BUILD_SHARED_LIBS)
    find_library(BULLET_LINEARMATH_LIB LinearMath PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    if(VPVL2_NO_BULLET)
      if(BULLET_INCLUDE_DIRS AND BULLET_LINEARMATH_LIB)
        target_link_libraries(${target} ${BULLET_LINEARMATH_LIB})
      else()
        message(FATAL_ERROR "Required LinearMath (included in BulletPhysics) is not found.")
      endif()
    else()
      find_library(BULLET_COLLISION_LIB BulletCollision PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
      find_library(BULLET_DYNAMICS_LIB BulletDynamics PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
      find_library(BULLET_SOFTBODY_LIB BulletSoftBody PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
      if(BULLET_COLLISION_LIB AND BULLET_DYNAMICS_LIB AND BULLET_SOFTBODY_LIB AND BULLET_LINEARMATH_LIB)
        target_link_libraries(${target} ${BULLET_COLLISION_LIB} ${BULLET_DYNAMICS_LIB} ${BULLET_SOFTBODY_LIB} ${BULLET_LINEARMATH_LIB})
      else()
        message(FATAL_ERROR "Required BulletPhysics is not found.")
      endif()
    endif()
  endif()
endfunction()

function(link_assimp target)
  get_build_type(ASSIMP_BUILD_TYPE)
  set(ASSIMP_LOCAL_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../assimp/${ASSIMP_BUILD_TYPE}/lib")
  set(ASSIMP_LOCAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../assimp/include")
  find_path(ASSIMP_INCLUDE_DIRS assimp.hpp PATHS ${ASSIMP_LOCAL_INCLUDE_DIR} $ENV{ASSIMP_INCLUDE_DIR})
  include_directories(${ASSIMP_INCLUDE_DIRS})
  if(BUILD_SHARED_LIBS)
    find_library(ASSIMP_LIBRARY assimp PATHS ${ASSIMP_LOCAL_LIBRARY_DIR} $ENV{ASSIMP_LIBRARY_DIR})
    if (ASSIMP_LIBRARY AND ASSIMP_LIBRARY)
      target_link_libraries(${target} ${ASSIMP_LIBRARY})
    else()
      message(FATAL_ERROR "Required Open Asset Import Library is not found.")
        endif()
  endif()
endfunction()

function(link_devil target)
  get_build_type(DEVIL_BUILD_TYPE)
  set(DEVIL_LOCAL_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../devil/${DEVIL_BUILD_TYPE}_native/lib")
  set(DEVIL_LOCAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../devil/include")
  find_path(DEVIL_INCLUDE_DIRS IL/il.h PATHS ${DEVIL_LOCAL_INCLUDE_DIR} $ENV{DEVIL_INCLUDE_DIR})
  find_library(DEVIL_IL_LIBRARY IL PATHS ${DEVIL_LOCAL_LIBRARY_DIR} $ENV{DEVIL_LIBRARY_DIR})
  find_library(DEVIL_ILU_LIBRARY ILU PATHS ${DEVIL_LOCAL_LIBRARY_DIR} $ENV{DEVIL_LIBRARY_DIR})
  find_library(DEVIL_ILUT_LIBRARY ILUT PATHS ${DEVIL_LOCAL_LIBRARY_DIR} $ENV{DEVIL_LIBRARY_DIR})
  if (DEVIL_INCLUDE_DIRS AND DEVIL_IL_LIBRARY AND DEVIL_ILU_LIBRARY AND DEVIL_ILUT_LIBRARY)
    target_link_libraries(${target} ${DEVIL_IL_LIBRARY} ${DEVIL_ILU_LIBRARY} ${DEVIL_ILUT_LIBRARY})
    include_directories(${DEVIL_INCLUDE_DIRS})
  else()
    message(FATAL_ERROR "Required DevIL is not found.")
  endif()
endfunction()

function(link_icu_or_iconv target)
  find_library(ICU_LIBRARY_IO icuio)
  find_library(ICU_LIBRARY_UC icuuc)
  find_path(ICU_INCLUDE_DIRS unicode/ucnv.h)
  if(ICU_LIBRARY_IO AND ICU_LIBRARY_UC AND ICU_INCLUDE_DIRS)
    add_definitions(-DVPVL2_HAS_ICU)
    target_link_libraries(${target} ${ICU_LIBRARY_IO} ${ICU_LIBRARY_UC})
    include_directories(${ICU_INCLUDE_DIRS})
  else()
    find_library(ICONV_LIBRARY iconv)
    if(ICONV_LIBRARY)
      add_definitions(-DVPVL2_HAS_ICONV)
      target_link_libraries(${target} ${ICONV_LIBRARY})
    endif()
  endif()
endfunction()

function(get_build_type var)
  string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
  if(MINGW)
    set(${var} "${CMAKE_BUILD_TYPE_TOLOWER}-mingw" PARENT_SCOPE)
  else()
    set(${var} ${CMAKE_BUILD_TYPE_TOLOWER} PARENT_SCOPE)
  endif()
endfunction()

# imported functions from Allegro5's cmake
function(append_lib_type_suffix var)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_TOLOWER)
  if(CMAKE_BUILD_TYPE_TOLOWER STREQUAL "debug")
    set(${var} "${${var}}_debug" PARENT_SCOPE)
  endif()
  if(CMAKE_BUILD_TYPE_TOLOWER MATCHES "profile")
    set(${var} "${${var}}_profile" PARENT_SCOPE)
  endif()
endfunction()

function(append_lib_linkage_suffix var)
  if(NOT BUILD_SHARED_LIBS)
    #set(${var} "${${var}}_static" PARENT_SCOPE)
  endif()
endfunction()
# end of imported

option(BUILD_SHARED_LIBS "Build Shared Libraries (default is OFF)" OFF)
if(BUILD_SHARED_LIBS)
  set(VPVL2_LIB_TYPE SHARED)
else()
  set(VPVL2_LIB_TYPE STATIC)
endif()

option(VPVL2_COORDINATE_OPENGL "Use OpenGL coordinate system (default is ON)" ON)
option(VPVL2_NO_BULLET "Build libvpvl2 without BulletPhysics except LinearMath (default is OFF)" OFF)

# intercept to add source
option(VPVL2_OPENGL_RENDERER "Include OpenGL renderer class (default is OFF)" OFF)
option(VPVL2_ENABLE_NVIDIA_CG "Include OpenGL renderer class using NVIDIA Cg (default is OFF)" OFF)
option(VPVL2_ENABLE_OPENCL "Enable OpenCL acceleration (default is OFF)" OFF)
if(VPVL2_OPENGL_RENDERER)
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/gl2 vpvl2_gl_sources)
  list(APPEND vpvl2_internal_headers
      ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/gl2/AssetRenderEngine.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/gl2/PMDRenderEngine.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/gl2/PMXRenderEngine.h
  )
  if(VPVL2_ENABLE_NVIDIA_CG)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/cg vpvl2_gl_sources)
    list(APPEND vpvl2_internal_headers
         ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cg/AssetRenderEngine.h
         ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cg/Effect.h
         ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cg/EffectEngine.h
         ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cg/PMDRenderEngine.h
         ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cg/PMXRenderEngine.h
    )
  endif()
  if(VPVL2_ENABLE_OPENCL)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/cl vpvl2_gl_sources)
    list(APPEND vpvl2_internal_headers
         ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cl/Context.h
         ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cl/PMDAccelerator.h
         ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cl/PMXAccelerator.h
     )
  endif()
  list(APPEND vpvl2_sources ${vpvl2_gl_sources})
  set(VPVL2_COORDINATE_OPENGL ON BOOL)
endif()

# library configuration
option(VPVL2_ENABLE_PROJECT "Include the class of loading and saving project files (default is OFF)" OFF)
if(VPVL2_ENABLE_PROJECT)
  list(APPEND vpvl2_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/core/project/Project.cc)
  list(APPEND vpvl2_public_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/Project.h)
endif()

# libvpvl sources
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/asset vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmd vpvl2_sources)

add_library(vpvl2 ${VPVL2_LIB_TYPE} ${vpvl2_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
set_target_properties(vpvl2 PROPERTIES VERSION ${VPVL2_VERSION} SOVERSION ${VPVL2_VERSION_COMPATIBLE})

# project include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# find libvpvl
link_vpvl(vpvl2)
list(APPEND vpvl2_public_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/asset/Model.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Bone.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Label.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Model.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Morph.h
)

# find Bullet Physics
link_bullet(vpvl2)

# find assimp library
option(VPVL2_LINK_ASSIMP "link against Open Asset Import Library (default is OFF)" OFF)
if(VPVL2_LINK_ASSIMP)
  link_assimp(vpvl2)
endif()

set(VPVL2_OUTPUT_NAME "vpvl2")
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
append_lib_type_suffix(VPVL2_OUTPUT_NAME)
append_lib_linkage_suffix(VPVL2_OUTPUT_NAME)
set_target_properties(vpvl2 PROPERTIES OUTPUT_NAME ${VPVL2_OUTPUT_NAME})

if(WIN32 AND BUILD_SHARED_LIBS)
  set_target_properties(vpvl2
      PROPERTIES PREFIX "" SUFFIX .${VPVL2_VERSION}.dll
      IMPORT_SUFFIX ${CMAKE_IMPORT_LIBRARY_SUFFIX})
endif()

# create as a framework if build on darwin environment
if(APPLE)
  if(BUILD_SHARED_LIBS AND FRAMEWORK)
    install(TARGETS vpvl2 DESTINATION .)
    set_target_properties(vpvl2 PROPERTIES FRAMEWORK true)
    set_target_properties(vpvl2 PROPERTIES PUBLIC_HEADER "${vpvl2_public_headers}")
  endif()
  set_target_properties(vpvl2 PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# link against Qt
option(VPVL2_LINK_QT "Link against Qt 4.8 (enabling VPVL2_OPENGL_RENDERER required, default is OFF)" OFF)
option(VPVL2_LINK_QT_WITH_OPENCV "Build a renderer program with recording feature using OpenCV (default is OFF)" OFF)
option(VPVL2_BUILD_QT_RENDERER "Build a renderer program using Qt 4.8 (enabling VPVL2_OPENGL_RENDERER required, default is OFF)" OFF)
if(VPVL2_OPENGL_RENDERER AND (VPVL2_BUILD_QT_RENDERER OR VPVL2_LINK_QT))
  find_package(Qt4 4.8 REQUIRED QtCore QtGui QtOpenGL)
  include(${QT_USE_FILE})
  target_link_libraries(vpvl2 ${QT_LIBRARIES})
  if(VPVL2_BUILD_QT_RENDERER)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/qt/common vpvl2qtcommon_sources)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/qt/unzip vpvl2qtcommon_sources)
    set(vpvl2qtcommon_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt/Archive.h
                              ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt/CString.h
                              ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt/CustomGLContext.h
                              ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt/DDSTexture.h
                              ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt/Delegate.h
                              ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt/Encoding.h
                              ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt/World.h)
    set(vpvl2_qt_sources ${CMAKE_CURRENT_SOURCE_DIR}/render/qt/main.cc
                         ${CMAKE_CURRENT_SOURCE_DIR}/render/qt/UI.cc)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/render/qt
                        ${CMAKE_CURRENT_SOURCE_DIR}/render/qt
                        ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt)
    add_library(vpvl2qtcommon ${vpvl2qtcommon_sources} ${vpvl2qtcommon_headers})
    target_link_libraries(vpvl2qtcommon vpvl2 ${QT_LIBRARIES})
    set(VPVL2QTCOMMON_OUTPUT_NAME "vpvl2qtcommon")
    append_lib_type_suffix(VPVL2QTCOMMON_OUTPUT_NAME)
    append_lib_linkage_suffix(VPVL2QTCOMMON_OUTPUT_NAME)
    set_target_properties(vpvl2qtcommon PROPERTIES OUTPUT_NAME ${VPVL2QTCOMMON_OUTPUT_NAME}
                                                   VERSION ${VPVL2_VERSION}
                                                   SOVERSION ${VPVL2_VERSION_COMPATIBLE})
    add_executable(vpvl2_qt ${vpvl2_qt_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
    target_link_libraries(vpvl2_qt vpvl2 vpvl2qtcommon ${QT_LIBRARIES})
    if(APPLE)
      add_definitions(-DUSE_FILE32API)
	  find_library(COCOA_LIBRARY Cocoa)
	  find_path(COCOA_INCLUDE_DIR Cocoa/Cocoa.h)
	  target_link_libraries(vpvl2qtcommon ${COCOA_LIBRARY})
	  include_directories(${COCOA_INCLUDE_DIR})
     endif()
  endif()
endif()

# link against OpenGL
if(VPVL2_OPENGL_RENDERER)
  find_package(OpenGL REQUIRED)
  target_link_libraries(vpvl2 ${OPENGL_gl_LIBRARY})
  include_directories(${OPENGL_INCLUDE_DIR})
  if(VPVL2_ENABLE_NVIDIA_CG)
    find_library(Cg_LIBRARY Cg)
    find_path(Cg_INCLUDE_DIR Cg/cg.h)
    if(NOT APPLE)
      find_library(CgGL_LIBRARY CgGL)
      target_link_libraries(vpvl2 ${Cg_LIBRARY} ${CgGL_LIBRARY})
    else()
      target_link_libraries(vpvl2 ${Cg_LIBRARY})
    endif()
    include_directories(${Cg_INCLUDE_DIR})
  endif()
  if(VPVL2_ENABLE_OPENCL)
    find_library(OPENCL_LIBRARY OpenCL)
    if(APPLE)
      find_path(OPENCL_INCLUDE_DIR OpenCL/cl.h)
    else()
      find_path(OPENCL_INCLUDE_DIR CL/cl.h)
    endif()
    target_link_libraries(vpvl2 ${OPENCL_LIBRARY})
    include_directories(${OPENCL_INCLUDE_DIR})
  endif()
endif()

# link against libxml2
if(VPVL2_ENABLE_PROJECT)
  find_package(LibXml2 REQUIRED)
  target_link_libraries(vpvl2 ${LIBXML2_LIBRARIES})
  include_directories(${LIBXML2_INCLUDE_DIR})
endif()

# link against DevIL
option(VPVL2_LINK_DEVIL "link against DevIL (default is OFF)" OFF)
if(VPVL2_BUILD_QT_RENDERER AND VPVL2_LINK_DEVIL)
  link_devil(vpvl2qtcommon)
endif()

# extra renderer program with SDL
option(VPVL2_BUILD_SDL "Build a renderer program using SDL to test rendering result (enabling VPVL2_OPENGL_RENDERER required, default is OFF)" OFF)
if(VPVL2_OPENGL_RENDERER AND VPVL2_BUILD_SDL)
  find_package(SDL REQUIRED)
  set(vpvl2_sdl_sources render/sdl/main.cc)
  if(APPLE)
    list(APPEND vpvl2_sdl_sources render/sdl/SDLMain.m render/sdl/SDLMain.h)
  endif()
  add_executable(vpvl2_sdl ${vpvl2_sdl_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
  target_link_libraries(vpvl2_sdl vpvl2 ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})
  include_directories(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})
  link_icu_or_iconv(vpvl2_sdl)
endif()

option(VPVL2_BUILD_IOS "Build a library for iOS with workaround (default is OFF)" OFF)
option(VPVL2_ENABLE_GLES2 "Build a library for GLES2 compliant (default is OFF)" OFF)

# generate configuration
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h")
set(vpvl2_public_headers ${vpvl2_public_headers} ${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
if(NOT WIN32)
  set(PKGCONFIG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig/" CACHE STRING "Base directory for pkgconfig files")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vpvl2.pc.in ${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc DESTINATION ${PKGCONFIG_INSTALL_PREFIX})
endif()

# install configuration
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")
  # disable _CRT_SECURE_NO_WARNINGS for surpressing warnings from vpvl2/Common.h
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
else()
  install(TARGETS vpvl2 DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2" DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
      PATTERN "*.h" PATTERN "config.h.in" EXCLUDE PATTERN "internal" EXCLUDE PATTERN ".svn" EXCLUDE)
endif()
