cmake_minimum_required(VERSION 2.8)

set(VPVL2_VERSION_MAJOR 0)
set(VPVL2_VERSION_COMPAT 7)
set(VPVL2_VERSION_MINOR 0)

# set library version
set(VPVL2_VERSION "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.${VPVL2_VERSION_MINOR}")
set(VPVL2_VERSION_COMPATIBLE "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.0")

# project configuration
project(libvpvl2)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/asset vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/base vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/internal vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/mvd vpvl2_sources)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmd vpvl2_sources)
#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmd2 vpvl2_sources)
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IMaterial.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IMorph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IMorphKeyframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IMotion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IRenderDelegate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IRenderEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IString.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/IVertex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/Scene.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/icu/String.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/icu/Encoding.h
)
set(vpvl2_internal_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/internal/util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/internal/BaseJoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/internal/BaseRigidBody.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/asset/Model.h
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

if(NOT CMAKE_BUILD_TYPE)
 set(CMAKE_BUILD_TYPE "Release")
endif()

# declaration of function
function(get_build_type var)
  if(MSVC)
      set(${var} "msvc-build" PARENT_SCOPE)
  else()
    string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
    if(MINGW)
      set(${var} "${CMAKE_BUILD_TYPE_TOLOWER}-mingw" PARENT_SCOPE)
    else()
      set(${var} ${CMAKE_BUILD_TYPE_TOLOWER} PARENT_SCOPE)
    endif()
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

function(link_bullet target)
  if(BUILD_SHARED_LIBS OR MSVC)
    if(VPVL2_NO_BULLET)
      target_link_libraries(${target} ${BULLET_LINEARMATH_LIB})
    else()
      target_link_libraries(${target} ${BULLET_COLLISION_LIB} ${BULLET_DYNAMICS_LIB} ${BULLET_SOFTBODY_LIB} ${BULLET_LINEARMATH_LIB})
        endif()
  endif()
endfunction()

function(find_bullet target)
  get_build_type(BULLET_BUILD_TYPE)
  set(BULLET_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bullet-src")
  set(BULLET_LIBRARY_LOCAL_DIR "${BULLET_SOURCE_DIRECTORY}/${BULLET_BUILD_TYPE}/lib")
  # find from MMDAI/bullet or environment variables
  find_path(BULLET_INCLUDE_DIR btBulletCollisionCommon.h PATHS "${BULLET_SOURCE_DIRECTORY}/src" $ENV{BULLET_INCLUDE_DIR})
  if(BULLET_INCLUDE_DIR)
    include_directories(${BULLET_INCLUDE_DIR})
  else()
    find_path(BULLET_INCLUDE_DIR bullet/btBulletCollisionCommon.h PATHS "${BULLET_SOURCE_DIRECTORY}/src" $ENV{BULLET_INCLUDE_DIR})
    include_directories(${BULLET_INCLUDE_DIR}/bullet)
  endif()
  if(BUILD_SHARED_LIBS OR MSVC)
    find_library(BULLET_LINEARMATH_LIB LinearMath PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    if(NOT VPVL2_NO_BULLET)
      find_library(BULLET_COLLISION_LIB BulletCollision PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
      find_library(BULLET_DYNAMICS_LIB BulletDynamics PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
      find_library(BULLET_SOFTBODY_LIB BulletSoftBody PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    endif()
  endif()
  link_bullet(${target})
endfunction()

function(link_assimp target)
  if((BUILD_SHARED_LIBS OR MSVC) AND VPVL2_LINK_ASSIMP)
    target_link_libraries(${target} ${ASSIMP_LIBRARY})
  endif()
endfunction()

function(find_assimp target)
  if(VPVL2_LINK_ASSIMP)
    get_build_type(ASSIMP_BUILD_TYPE)
    set(ASSIMP_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../assimp-src")
    set(ASSIMP_LOCAL_LIBRARY_DIR "${ASSIMP_SOURCE_DIRECTORY}/${ASSIMP_BUILD_TYPE}/code")
    set(ASSIMP_LOCAL_INCLUDE_DIR "${ASSIMP_SOURCE_DIRECTORY}/include")
    find_path(ASSIMP_INCLUDE_DIR assimp.hpp PATHS ${ASSIMP_LOCAL_INCLUDE_DIR} $ENV{ASSIMP_INCLUDE_DIR})
    if(ASSIMP_INCLUDE_DIR)
      include_directories(${ASSIMP_INCLUDE_DIR})
    else()
      find_path(ASSIMP_INCLUDE_DIR assimp/assimp.hpp PATHS ${ASSIMP_LOCAL_INCLUDE_DIR} $ENV{ASSIMP_INCLUDE_DIR})
      include_directories(${ASSIMP_INCLUDE_DIR}/assimp)
    endif()
    if(BUILD_SHARED_LIBS OR MSVC)
      find_library(ASSIMP_LIBRARY assimp PATHS ${ASSIMP_LOCAL_LIBRARY_DIR} $ENV{ASSIMP_LIBRARY_DIR})
    endif()
    link_assimp(${target})
  endif()
endfunction()

function(link_vpvl target)
  if((BUILD_SHARED_LIBS OR MSVC) AND VPVL2_LINK_VPVL)
    target_link_libraries(${target} ${VPVL_LIBRARY})
  endif()
endfunction()

function(find_vpvl target)
  if(VPVL2_LINK_VPVL)
    get_build_type(VPVL_BUILD_TYPE)
    set(VPVL_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../libvpvl")
    set(VPVL_LOCAL_BUILD_DIR "${VPVL_SOURCE_DIRECTORY}/${VPVL_BUILD_TYPE}")
    set(VPVL_LOCAL_INCLUDE_DIR "${VPVL_SOURCE_DIRECTORY}/include")
    find_path(VPVL_INCLUDE_DIR vpvl/vpvl.h PATHS ${VPVL_LOCAL_INCLUDE_DIR} $ENV{VPVL_INCLUDE_DIR})
    find_path(VPVL_CONFIG_DIR vpvl/config.h PATHS "${VPVL_LOCAL_BUILD_DIR}/include" $ENV{VPVL_CONFIG_DIR})
    include_directories(${VPVL_INCLUDE_DIR} ${VPVL_CONFIG_DIR})
    if(BUILD_SHARED_LIBS OR MSVC)
      set(VPVL_LIBRARY_NAME vpvl)
      append_lib_type_suffix(VPVL_LIBRARY_NAME)
      find_library(VPVL_LIBRARY ${VPVL_LIBRARY_NAME} PATHS "${VPVL_LOCAL_BUILD_DIR}/lib" $ENV{VPVL_LIBRARY_DIR})
    endif()
    link_vpvl(${target})
  endif()
endfunction()

function(link_glew target)
  if(VPVL2_OPENGL_RENDERER AND VPVL2_LINK_GLEW)
    target_link_libraries(${target} ${GLEW_LIBRARY})
    if(MSVC)
      target_link_libraries(${target} ${GLEW_MX_LIBRARY})
    endif()
  endif()
endfunction()

function(find_glew target)
  if(VPVL2_OPENGL_RENDERER AND VPVL2_LINK_GLEW)
    if(BUILD_SHARED_LIBS)
      find_library(GLEW_LIBRARY glew glew32 PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../glew-src/lib)
    else()
      find_library(GLEW_LIBRARY glew glew32s PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../glew-src/lib)
    endif()
    if(MSVC)
      if(BUILD_SHARED_LIBS)
        find_library(GLEW_MX_LIBRARY glew32mx PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../glew-src/lib)
      else()
        find_library(GLEW_MX_LIBRARY glew32mxs PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../glew-src/lib)
      endif()
    endif()
    find_path(GLEW_INCLUDE_DIR GL/glew.h PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../glew-src/include)
    include_directories(${GLEW_INCLUDE_DIR})
    link_glew(${target})
  endif()
endfunction()

function(link_devil target)
  if(VPVL2_BUILD_QT_RENDERER AND VPVL2_LINK_DEVIL)
    get_build_type(DEVIL_BUILD_TYPE)
    set(DEVIL_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../devil-src")
    set(DEVIL_LOCAL_LIBRARY_DIR "${DEVIL_SOURCE_DIRECTORY}/${DEVIL_BUILD_TYPE}_native/lib")
    set(DEVIL_LOCAL_INCLUDE_DIR "${DEVIL_SOURCE_DIRECTORY}/include")
    find_path(DEVIL_INCLUDE_DIR IL/il.h PATHS ${DEVIL_LOCAL_INCLUDE_DIR} $ENV{DEVIL_INCLUDE_DIR})
    find_library(DEVIL_IL_LIBRARY IL PATHS ${DEVIL_LOCAL_LIBRARY_DIR} $ENV{DEVIL_LIBRARY_DIR})
    find_library(DEVIL_ILU_LIBRARY ILU PATHS ${DEVIL_LOCAL_LIBRARY_DIR} $ENV{DEVIL_LIBRARY_DIR})
    find_library(DEVIL_ILUT_LIBRARY ILUT PATHS ${DEVIL_LOCAL_LIBRARY_DIR} $ENV{DEVIL_LIBRARY_DIR})
    if (DEVIL_INCLUDE_DIR AND DEVIL_IL_LIBRARY AND DEVIL_ILU_LIBRARY AND DEVIL_ILUT_LIBRARY)
      target_link_libraries(${target} ${DEVIL_IL_LIBRARY} ${DEVIL_ILU_LIBRARY} ${DEVIL_ILUT_LIBRARY})
      include_directories(${DEVIL_INCLUDE_DIR})
    else()
      message(FATAL_ERROR "Required DevIL is not found.")
    endif()
  endif()
endfunction()

function(link_icu_or_iconv target)
  find_library(ICU_LIBRARY_I18N icui18n)
  find_library(ICU_LIBRARY_UC icuuc)
  find_path(ICU_INCLUDE_DIR unicode/unistr.h)
  if(ICU_LIBRARY_I18N AND ICU_LIBRARY_UC AND ICU_INCLUDE_DIR)
    add_definitions(-DVPVL2_HAS_ICU)
    target_link_libraries(${target} ${ICU_LIBRARY_I18N} ${ICU_LIBRARY_IO} ${ICU_LIBRARY_UC})
    include_directories(${ICU_INCLUDE_DIR})
  else()
    find_library(ICONV_LIBRARY iconv)
    if(ICONV_LIBRARY)
      add_definitions(-DVPVL2_HAS_ICONV)
      target_link_libraries(${target} ${ICONV_LIBRARY})
    endif()
  endif()
endfunction()

function(find_nvtt target)
  if(VPVL2_OPENGL_RENDERER AND VPVL2_LINK_NVTT)
    get_build_type(NVTT_BUILD_TYPE)
    set(NVTT_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../nvtt-src")
    set(NVTT_INCLUDE_DIRECTORY "${NVTT_SOURCE_DIRECTORY}/src")
    set(NVTT_LOCAL_LIBRARY_DIR "${NVTT_SOURCE_DIRECTORY}/build-${NVTT_BUILD_TYPE}/lib")
    find_path(NVTT_CONFIG_INCLUDE_DIR nvconfig.h PATHS "${NVTT_LOCAL_LIBRARY_DIR}/../src" $ENV{NVTT_CONFIG_INCLUDE_DIR})
    find_path(NVTT_BASE_INCLUDE_DIR nvcore/nvcore.h PATHS ${NVTT_INCLUDE_DIRECTORY} $ENV{NVTT_BASE_INCLUDE_DIR})
    find_path(NVTT_POSH_INCLUDE_DIR posh.h PATHS "${NVTT_SOURCE_DIRECTORY}/extern/poshlib" $ENV{NVTT_POSHLIB_INCLUDE_DIR})
    find_library(NVTT_NVCORE_LIBRARY nvcore PATHS ${NVTT_LOCAL_LIBRARY_DIR} $ENV{NVTT_LIBRARY_DIR})
    find_library(NVTT_NVIMAGE_LIBRARY nvimage PATHS ${NVTT_LOCAL_LIBRARY_DIR} $ENV{NVTT_LIBRARY_DIR})
    find_library(NVTT_NVMATH_LIBRARY nvmath PATHS ${NVTT_LOCAL_LIBRARY_DIR} $ENV{NVTT_LIBRARY_DIR})
    if (NVTT_CONFIG_INCLUDE_DIR AND NVTT_BASE_INCLUDE_DIR AND NVTT_POSH_INCLUDE_DIR)
      include_directories(${NVTT_CONFIG_INCLUDE_DIR} ${NVTT_BASE_INCLUDE_DIR} ${NVTT_POSH_INCLUDE_DIR})
    else()
      message(FATAL_ERROR "Required nvtt is not found.")
    endif()
  endif()
endfunction()

function(link_nvtt target)
  if((BUILD_SHARED_LIBS OR MSVC) AND VPVL2_LINK_NVTT)
    target_link_libraries(${target} ${NVTT_NVCORE_LIBRARY} ${NVTT_NVIMAGE_LIBRARY} ${NVTT_NVMATH_LIBRARY})
  endif()
endfunction()
# end of imported

# options
option(BUILD_SHARED_LIBS "Build Shared Libraries (default is OFF)" OFF)
option(VPVL2_LINK_QT "Link against Qt 4.8 (enabling VPVL2_OPENGL_RENDERER required, default is OFF)" OFF)
option(VPVL2_LINK_QT_WITH_OPENCV "Build a renderer program with recording feature using OpenCV (default is OFF)" OFF)
option(VPVL2_BUILD_QT_RENDERER "Build a renderer program using Qt 4.8 (enabling VPVL2_OPENGL_RENDERER is required, default is OFF)" OFF)
option(VPVL2_COORDINATE_OPENGL "Use OpenGL coordinate system (default is ON)" ON)
option(VPVL2_NO_BULLET "Build libvpvl2 without BulletPhysics except LinearMath (default is OFF)" OFF)
option(VPVL2_OPENGL_RENDERER "Include OpenGL renderer class (default is OFF)" OFF)
option(VPVL2_ENABLE_NVIDIA_CG "Include OpenGL renderer class using NVIDIA Cg (default is OFF)" OFF)
option(VPVL2_ENABLE_OPENCL "Enable OpenCL acceleration (default is OFF)" OFF)
option(VPVL2_ENABLE_PROJECT "Include the class of loading and saving project files (default is OFF)" OFF)
option(VPVL2_LINK_SDL1 "Enable linking against SDL 1.2 (enabling VPVL2_OPENGL_RENDERER is required, default is OFF)" OFF)
option(VPVL2_LINK_SDL2 "Enable linking against SDL 2.0 (enabling VPVL2_OPENGL_RENDERER is required, default is OFF)" OFF)
option(VPVL2_LINK_ASSIMP "link against Open Asset Import Library (default is OFF)" OFF)
option(VPVL2_LINK_DEVIL "link against DevIL (default is OFF)" OFF)
option(VPVL2_LINK_GLEW "link against GLEW (enabling VPVL2_OPENGL_RENDERER is required, default is OFF)" OFF)
option(VPVL2_LINK_NVTT "link against NVIDIA Texture Tools a.k.a nvtt (enabling VPVL2_OPENGL_RENDERER is required, default is OFF)" OFF)
option(VPVL2_LINK_VPVL "link against libvpvl (default is ON)" ON)
option(VPVL2_BUILD_IOS "Build a library for iOS with workaround (default is OFF)" OFF)
option(VPVL2_ENABLE_GLES2 "Build a library for GLES2 compliant (enabling VPVL2_OPENGL_RENDERER is required default is OFF)" OFF)

if(BUILD_SHARED_LIBS)
  set(VPVL2_LIB_TYPE SHARED)
else()
  set(VPVL2_LIB_TYPE STATIC)
endif()

# intercept to add source (OpenGL)
if(VPVL2_OPENGL_RENDERER)
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/gl2 vpvl2_gl_sources)
  list(APPEND vpvl2_internal_headers
      ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/internal/BaseRenderEngine.h
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

# intercept to add source (libvpvl)
if(VPVL2_LINK_VPVL)
  list(APPEND vpvl2_internal_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Bone.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Label.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Morph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/Vertex.h
  )
else()
  list(APPEND vpvl2_internal_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/Bone.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/Joint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/Label.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/Material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/Model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/Morph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/RigidBody.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/Vertex.h
  )
endif()

# intercept to add source (Project)
if(VPVL2_ENABLE_PROJECT)
  list(APPEND vpvl2_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/core/project/Project.cc)
  list(APPEND vpvl2_public_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/Project.h)
endif()

# generate configuration file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h")
set(vpvl2_public_headers ${vpvl2_public_headers} ${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h)

# create libvpvl2 project
add_library(vpvl2 ${VPVL2_LIB_TYPE} ${vpvl2_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
set_target_properties(vpvl2 PROPERTIES VERSION ${VPVL2_VERSION} SOVERSION ${VPVL2_VERSION_COMPATIBLE})

# project include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# find libvpvl
find_bullet(vpvl2)
find_assimp(vpvl2)
find_glew(vpvl2)
find_nvtt(vpvl2)
find_vpvl(vpvl2)

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
    if(APPLE)
      add_definitions(-DUSE_FILE32API)
      find_library(COCOA_LIBRARY Cocoa)
      find_path(COCOA_INCLUDE_DIR Cocoa/Cocoa.h)
      include_directories(${COCOA_INCLUDE_DIR})
      aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/qt/osx vpvl2qtcommon_sources)
    endif()
    add_library(vpvl2qtcommon ${vpvl2qtcommon_sources} ${vpvl2qtcommon_headers})
    link_nvtt(vpvl2qtcommon)
    if(APPLE)
      target_link_libraries(vpvl2qtcommon ${COCOA_LIBRARY})
    endif()
    target_link_libraries(vpvl2qtcommon vpvl2 ${QT_LIBRARIES})
    set(VPVL2QTCOMMON_OUTPUT_NAME "vpvl2qtcommon")
    append_lib_type_suffix(VPVL2QTCOMMON_OUTPUT_NAME)
    append_lib_linkage_suffix(VPVL2QTCOMMON_OUTPUT_NAME)
    set_target_properties(vpvl2qtcommon PROPERTIES OUTPUT_NAME ${VPVL2QTCOMMON_OUTPUT_NAME}
                                                   VERSION ${VPVL2_VERSION}
                                                   SOVERSION ${VPVL2_VERSION_COMPATIBLE})
    add_executable(vpvl2_qt ${vpvl2_qt_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
    target_link_libraries(vpvl2_qt vpvl2 vpvl2qtcommon ${QT_LIBRARIES})
  endif()
endif()

# link against OpenGL
if(VPVL2_OPENGL_RENDERER)
  if(VPVL2_PLATFORM_NACL)
    find_library(OPENGL_gl_LIBRARY ppapi_gles2)
    find_path(OPENGL_INCLUDE_DIR GLES2/gl2.h)
  else()
    find_package(OpenGL REQUIRED)
  endif()
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

# extra renderer program with SDL
if(VPVL2_OPENGL_RENDERER AND (VPVL2_LINK_SDL1 OR VPVL2_LINK_SDL2))
  if(VPVL2_LINK_SDL2)
    find_library(SDLIMAGE_LIBRARY SDL2_image REQUIRED)
    find_program(SDL2_CONFIG_SCRIPT sdl2-config REQUIRED)
    exec_program(${SDL2_CONFIG_SCRIPT} ARGS --cflags OUTPUT_VARIABLE SDL2_CFLAGS)
    exec_program(${SDL2_CONFIG_SCRIPT} ARGS --static-libs OUTPUT_VARIABLE SDL_LIBRARY)
    string(REGEX MATCHALL "-I[^ ]*" SDL2_INCLUDE_DIR ${SDL2_CFLAGS})
    string(REGEX MATCHALL "-D[^ ]*" SDL2_DEFINITIONS ${SDL2_CFLAGS})
    string(REPLACE "-I" "" SDL2_INCLUDE_DIR "${SDL2_INCLUDE_DIR}")
    add_definitions(${SDL2_DEFINITIONS})
    include_directories(${SDL2_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})
  elseif(VPVL2_LINK_SDL1)
    find_package(SDL REQUIRED)
    find_package(SDL_image REQUIRED)
    include_directories(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})
    if(APPLE)
      list(APPEND vpvl2_sdl_sources render/sdl/SDLMain.m render/sdl/SDLMain.h)
    endif()
  endif()
  find_path(GLM_INCLUDE_DIR glm/glm.hpp PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../glm-src)
  set(vpvl2_sdl_sources render/sdl/main.cc)
  add_executable(vpvl2_sdl ${vpvl2_sdl_sources} ${vpvl2_public_headers} ${vpvl2_internal_headers})
  target_link_libraries(vpvl2_sdl vpvl2 ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})
  include_directories(${GLM_INCLUDE_DIR})
  link_assimp(vpvl2_sdl)
  link_bullet(vpvl2_sdl)
  link_glew(vpvl2_sdl)
  link_icu_or_iconv(vpvl2_sdl)
  link_nvtt(vpvl2_sdl)
endif()

# generate configuration
if(NOT WIN32)
  set(PKGCONFIG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig/" CACHE STRING "Base directory for pkgconfig files")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vpvl2.pc.in ${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc DESTINATION ${PKGCONFIG_INSTALL_PREFIX})
endif()

# install configuration
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4251 /wd4819")
  # disable _CRT_SECURE_NO_WARNINGS for surpressing warnings from vpvl2/Common.h
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
else()
  install(TARGETS vpvl2 DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
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
          PATTERN "gl2" EXCLUDE
          PATTERN "qt" EXCLUDE)
  if(VPVL2_LINK_QT)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt" DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vpvl2")
  endif()
endif()

