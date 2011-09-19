cmake_minimum_required(VERSION 2.6)

set(VPVL_VERSION_MAJOR 0)
set(VPVL_VERSION_COMPAT 3)
set(VPVL_VERSION_MINOR 0)

# set library version
set(VPVL_VERSION "${VPVL_VERSION_MAJOR}.${VPVL_VERSION_COMPAT}.${VPVL_VERSION_MINOR}")
set(VPVL_VERSION_COMPATIBLE "${VPVL_VERSION_MAJOR}.${VPVL_VERSION_COMPAT}.0")

# project configuration
project(libvpvl)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src vpvl_sources)
set(vpvl_public_headers
    include/vpvl/Asset.h
    include/vpvl/BaseAnimation.h
    include/vpvl/BaseKeyFrame.h
    include/vpvl/BoneKeyFrame.h
    include/vpvl/Bone.h
    include/vpvl/BoneAnimation.h
    include/vpvl/CameraKeyFrame.h
    include/vpvl/CameraAnimation.h
    include/vpvl/Common.h
    include/vpvl/Constraint.h
    include/vpvl/Face.h
    include/vpvl/FaceKeyFrame.h
    include/vpvl/FaceAnimation.h
    include/vpvl/IK.h
    include/vpvl/Material.h
    include/vpvl/PMDModel.h
    include/vpvl/RigidBody.h
    include/vpvl/Scene.h
    include/vpvl/Vertex.h
    include/vpvl/VMDMotion.h
    include/vpvl/vpvl.h
)
set(vpvl_internal_headers
    include/vpvl/internal/util.h
)

IF(NOT CMAKE_BUILD_TYPE)
 SET(CMAKE_BUILD_TYPE "Release")
ENDIF()

# declarations of function
function(link_bullet target)
  OPTION(FIND_BULLET_BY_PKGCONFIG "Find Bullet Physics by pkg-config" OFF)
  if(FIND_BULLET_BY_PKGCONFIG)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(BULLET REQUIRED bullet)
  endif()
  # exists bullet in pkgconfig?
  if(BULLET_FOUND)
    include_directories(${BULLET_INCLUDE_DIRS})
    target_link_libraries(${target} ${BULLET_LIBRARIES})
    link_directories(${BULLET_LIBRARY_DIRS})
  else()
    get_build_type(BULLET_BUILD_TYPE)
    if(MSVC)
      set(BULLET_LIBRARY_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bullet/msvc-build/lib/${BULLET_BUILD_TYPE}")
    else()
      set(BULLET_LIBRARY_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bullet/${BULLET_BUILD_TYPE}/lib")
    endif()
    # find from MMDAI/bullet or environment variables
    find_path(BULLET_INCLUDE_DIRS btBulletDynamicsCommon.h PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../bullet/src" $ENV{BULLET_INCLUDE_DIR})
    find_library(BULLET_COLLISION_LIB BulletCollision PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_DYNAMICS_LIB BulletDynamics PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_SOFTBODY_LIB BulletSoftBody PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_LINEARMATH_LIB LinearMath PATHS ${BULLET_LIBRARY_LOCAL_DIR} $ENV{BULLET_LIBRARY_DIR})
    if(VPVL_NO_BULLET)
      if(BULLET_INCLUDE_DIRS AND BULLET_LINEARMATH_LIB)
        include_directories(${BULLET_INCLUDE_DIRS})
        target_link_libraries(${target} ${BULLET_LINEARMATH_LIB})
      else()
        message(FATAL_ERROR "Required LinearMath (included in BulletPhysics) is not found.")
      endif()
    else()
      if(BULLET_INCLUDE_DIRS AND BULLET_COLLISION_LIB AND BULLET_DYNAMICS_LIB AND BULLET_SOFTBODY_LIB AND BULLET_LINEARMATH_LIB)
        include_directories(${BULLET_INCLUDE_DIRS})
        target_link_libraries(${target} ${BULLET_COLLISION_LIB} ${BULLET_DYNAMICS_LIB} ${BULLET_SOFTBODY_LIB} ${BULLET_LINEARMATH_LIB})
      else()
        message(FATAL_ERROR "Required BulletPhysics is not found.")
      endif()
    endif()
  endif()
endfunction()

function(link_glew target)
  find_path(GLEW_INCLUDE GL/glew.h PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../glew/include" $ENV{GLEW_DIR}/include)
  find_library(GLEW_LIBRARY NAMES GLEW glew32 PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../glew/lib" $ENV{GLEW_DIR}/lib)
  if(GLEW_INCLUDE AND GLEW_LIBRARY)
    target_link_libraries(${target} ${GLEW_LIBRARY})
    include_directories(${GLEW_INCLUDE})
  else()
    message(FATAL_ERROR "Required OpenGL GLEW is not found.")
  endif()
endfunction()

function(link_assimp target)
  set(ASSIMP_LOCAL_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../assimp/lib")
  set(ASSIMP_LOCAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../assimp/include")
  find_path(ASSIMP_INCLUDE_DIRS assimp.hpp PATHS ${ASSIMP_LOCAL_INCLUDE_DIR} $ENV{ASSIMP_INCLUDE_DIR})
  find_library(ASSIMP_LIBRARY assimp PATHS ${ASSIMP_LOCAL_LIBRARY_DIR} $ENV{ASSIMP_LIBRARY_DIR})
  if (ASSIMP_LIBRARY AND ASSIMP_LIBRARY)
    target_link_libraries(${target} ${ASSIMP_LIBRARY})
    include_directories(${ASSIMP_INCLUDE_DIRS})
  else()
    message(FATAL_ERROR "Required Open Asset Import Library is not found.")
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
  set(VPVL_LIB_TYPE SHARED)
else()
  set(VPVL_LIB_TYPE STATIC)
endif()

option(VPVL_COORDINATE_OPENGL "Use OpenGL coordinate system (default is ON)" ON)
option(VPVL_USE_ALLEGRO5 "Use Allegro5 OpenGL extensions instead of GLEW (default is OFF)" OFF)
option(VPVL_NO_BULLET "Build libvpvl without BulletPhysics except LinearMath (default is OFF)" OFF)

# intercept to add source
option(VPVL_OPENGL_RENDERER "Include OpenGL renderer class (default is OFF)" OFF)
if(VPVL_OPENGL_RENDERER)
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/engine/gl vpvl_gl_sources)
  set(VPVL_COORDINATE_OPENGL ON BOOL)
  list(APPEND vpvl_sources ${vpvl_gl_sources})
  list(APPEND vpvl_public_headers include/vpvl/gl/Renderer.h)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl/config.h")
set(vpvl_public_headers ${vpvl_public_headers} ${CMAKE_CURRENT_BINARY_DIR}/include/vpvl/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
if(NOT WIN32)
  set(PKGCONFIG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig/" CACHE STRING "Base directory for pkgconfig files")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/vpvl.pc.in ${CMAKE_CURRENT_BINARY_DIR}/vpvl.pc @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/vpvl.pc DESTINATION ${PKGCONFIG_INSTALL_PREFIX})
endif()

add_library(vpvl ${VPVL_LIB_TYPE} ${vpvl_sources} ${vpvl_public_headers} ${vpvl_internal_headers})
set_target_properties(vpvl PROPERTIES VERSION ${VPVL_VERSION} SOVERSION ${VPVL_VERSION_COMPATIBLE})

# project include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# find Bullet Physics
link_bullet(vpvl)

# find assimp library
option(VPVL_LINK_ASSIMP "Link with Open Asset Import Library (default is OFF)" OFF)
if(VPVL_LINK_ASSIMP)
  link_assimp(vpvl)
endif()

# find Allegro5 game library if enabled
if(VPVL_USE_ALLEGRO5)
  find_path(ALLEG5_INCLUDE_DIRS allegro5/allegro5.h PATHS $ENV{ALLEG5_INCLUDE_DIR})
  find_library(ALLEG5_LIBRARY allegro PATHS $ENV{ALLEG5_LIBRARY_DIR})
  target_link_libraries(vpvl ${ALLEG5_LIBRARY})
  include_directories(${ALLEG5_INCLUDE_DIRS})
endif()

set(VPVL_OUTPUT_NAME "vpvl")
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
append_lib_type_suffix(VPVL_OUTPUT_NAME)
append_lib_linkage_suffix(VPVL_OUTPUT_NAME)
set_target_properties(vpvl PROPERTIES OUTPUT_NAME ${VPVL_OUTPUT_NAME})

if(WIN32 AND BUILD_SHARED_LIBS)
  set_target_properties(vpvl
      PROPERTIES PREFIX "" SUFFIX .${VPVL_VERSION}.dll
      IMPORT_SUFFIX ${CMAKE_IMPORT_LIBRARY_SUFFIX})
endif()

# create as a framework if build on darwin environment
if(APPLE)
  if(BUILD_SHARED_LIBS AND FRAMEWORK)
    install(TARGETS vpvl DESTINATION .)
    set_target_properties(vpvl PROPERTIES FRAMEWORK true)
    set_target_properties(vpvl PROPERTIES PUBLIC_HEADER "${vpvl_public_headers}")
  endif()
  set_target_properties(vpvl PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")
  # disable _CRT_SECURE_NO_WARNINGS for surpressing warnings from vpvl/Common.h
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
else()
  install(TARGETS vpvl DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl" DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
      PATTERN "*.h" PATTERN "config.h.in" EXCLUDE PATTERN "internal" EXCLUDE PATTERN ".svn" EXCLUDE)
endif()

# link with OpenGL and GLEW
if(VPVL_OPENGL_RENDERER)
  find_package(OpenGL REQUIRED)
  target_link_libraries(vpvl ${OPENGL_LIBRARIES})
  include_directories(${OPENGL_INCLUDE_DIR})
  if(NOT VPVL_USE_ALLEGRO5)
    link_glew(vpvl)
  endif()
endif()

# extra renderer program with SDL
option(VPVL_BUILD_SDL "Build a renderer program using SDL to test rendering result (enabling VPVL_OPENGL_RENDERER required, exclusive VPVL_USE_ALLEGRO5, default is OFF)" OFF)
if(VPVL_OPENGL_RENDERER AND VPVL_BUILD_SDL AND NOT VPVL_USE_ALLEGRO5)
  find_package(SDL REQUIRED)
  find_package(SDL_image REQUIRED)
  aux_source_directory(render/common vpvl_sdl_common_sources)
  if(APPLE)
    aux_source_directory(render/osx vpvl_sdl_osx_sources)
  endif()
  add_executable(vpvl_sdl ${vpvl_sdl_common_sources} ${vpvl_sdl_osx_sources} ${vpvl_public_headers} ${vpvl_internal_headers})
  target_link_libraries(vpvl_sdl vpvl ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})
  include_directories(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})
  find_library(ICU_LIBRARY_IO icuio)
  find_library(ICU_LIBRARY_UC icuuc)
  find_path(ICU_INCLUDE_DIRS unicode/ucnv.h)
  if(ICU_LIBRARY_IO AND ICU_LIBRARY_UC AND ICU_INCLUDE_DIRS)
      add_definitions(-DVPVL_HAS_ICU)
      target_link_libraries(vpvl_sdl ${ICU_LIBRARY_IO} ${ICU_LIBRARY_UC})
      include_directories(${ICU_INCLUDE_DIRS})
  else()
    find_library(ICONV_LIBRARY iconv)
    if(ICONV_LIBRARY)
      add_definitions(-DVPVL_HAS_ICONV)
      target_link_libraries(vpvl_sdl ${ICONV_LIBRARY})
    endif()
  endif()
endif()

# extra renderer program with allegro5
option(VPVL_BUILD_A5 "Build a renderer program using Allegro, OpenCV and NyARToolKit to test rendering result (enabling VPVL_OPENGL_RENDERER required, default is OFF)" OFF)
if(VPVL_OPENGL_RENDERER AND VPVL_USE_ALLEGRO5 AND VPVL_BUILD_A5)
  find_package(OpenCV REQUIRED)
  find_library(ALLEG5_LIBRARY_MAIN allegro_main PATHS $ENV{ALLEG5_LIBRARY_DIR})
  find_library(ALLEG5_LIBRARY_IMAGE allegro_image PATHS $ENV{ALLEG5_LIBRARY_DIR})
  aux_source_directory(render/a5 vpvl_a5_sources)
  add_executable(vpvl_a5 ${vpvl_a5_sources} ${vpvl_public_headers} ${vpvl_internal_headers})
  target_link_libraries(vpvl_a5 vpvl ${ALLEG5_LIBRARY_MAIN} ${ALLEG5_LIBRARY_IMAGE} opencv_core opencv_highgui)
endif()
