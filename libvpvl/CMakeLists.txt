cmake_minimum_required(VERSION 2.6)

# set library version
set(VPVL_VERSION 0.1.0)
set(VPVL_VERSION_COMPATIBLE 0.1.0)

# project configuration
project(libvpvl)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src vpvl_sources)
set(vpvl_public_headers
    include/vpvl/BaseMotion.h
    include/vpvl/Bone.h
    include/vpvl/BoneMotion.h
    include/vpvl/CameraMotion.h
    include/vpvl/Constraint.h
    include/vpvl/Face.h
    include/vpvl/FaceMotion.h
    include/vpvl/IK.h
    include/vpvl/Material.h
    include/vpvl/PMDModel.h
    include/vpvl/RigidBody.h
    include/vpvl/Scene.h
    include/vpvl/Vertex.h
    include/vpvl/VMDMotion.h
    include/vpvl/VPDPose.h
    include/vpvl/XMaterial.h
    include/vpvl/XModel.h
    include/vpvl/common.h
    include/vpvl/config.h
    include/vpvl/vpvl.h
)
set(vpvl_internal_headers
    include/vpvl/internal/BoneKeyFrame.h
    include/vpvl/internal/CameraKeyFrame.h
    include/vpvl/internal/FaceKeyFrame.h
    include/vpvl/internal/util.h
)

# declarations of function
function(link_bullet target)
  OPTION(FIND_BULLET_BY_PKGCONFIG "Find Bullet Physics by pkg-config" OFF)
  if(FIND_BULLET_BY_PKGCONFIG)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(BULLET REQUIRED bullet)
  endif()
  if(BULLET_FOUND)
    include_directories(${BULLET_INCLUDE_DIRS})
    target_link_libraries(${target} ${BULLET_LIBRARIES})
    link_directories(${BULLET_LIBRARY_DIRS})
  else()
    find_path(BULLET_INCLUDE_DIRS btBulletDynamicsCommon.h PATH $ENV{BULLET_INCLUDE_DIR})
    find_library(BULLET_COLLISION_LIB BulletCollision PATH $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_DYNAMICS_LIB BulletDynamics PATH $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_SOFTBODY_LIB BulletSoftBody PATH $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_LINEARMATH_LIB LinearMath PATH $ENV{BULLET_LIBRARY_DIR})
    if(BULLET_INCLUDE_DIRS AND BULLET_LIBRARIES)
      include_directories(${BULLET_INCLUDE_DIRS})
      target_link_libraries(${target} ${BULLET_COLLISION_LIB} ${BULLET_DYNAMICS_LIB} ${BULLET_SOFTBODY_LIB} ${BULLET_LINEARMATH_LIB})
    else()
      message(FATAL_ERROR "Required Bullet Physics is not found.")
    endif()
  endif()
endfunction()

function(link_glew target)
  find_path(GLEW_INCLUDE GL/glew.h PATH $ENV{GLEW_DIR})
  find_library(GLEW_LIBRARY GLEW PATH $ENV{GLEW_DIR})
  if(GLEW_INCLUDE AND GLEW_LIBRARY)
    target_link_libraries(${target} ${GLEW_LIBRARY})
    include_directories(${GLEW_INCLUDE})
  else()
    message(FATAL_ERROR "Required OpenGL GLEW is not found.")
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
    set(${var} "${${var}}_static" PARENT_SCOPE)
  endif()
endfunction()
# end of imported

option(BUILD_SHARED_LIBS "Build Shared Libraries (default is OFF)" OFF)
if(BUILD_SHARED_LIBS)
  set(VPVL_LIB_TYPE SHARED)
else()
  set(VPVL_LIB_TYPE STATIC)
endif()

option(VPVL_COORDINATE_OPENGL "Use OpenGL coordinate system (default is OFF)" OFF)
option(VPVL_USE_ALLEGRO5 "Use Allegro5 OpenGL extensions instead of GLEW (default is OFF)" OFF)

# intercept to add source
option(VPVL_OPENGL_RENDERER "Include OpenGL renderer class (default is OFF)" OFF)
if(VPVL_OPENGL_RENDERER)
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/engine/gl vpvl_gl_sources)
  set(VPVL_COORDINATE_OPENGL ON BOOL)
  list(APPEND vpvl_sources ${vpvl_gl_sources})
  list(APPEND vpvl_public_headers include/vpvl/gl/Renderer.h)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl/config.h.cmake
               ${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl/config.h)

add_library(vpvl ${VPVL_LIB_TYPE} ${vpvl_sources} ${vpvl_public_headers} ${vpvl_internal_headers})
set_target_properties(vpvl PROPERTIES VERSION ${VPVL_VERSION} SOVERSION ${VPVL_VERSION_COMPATIBLE})

# project include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# find Bullet Physics
link_bullet(vpvl)

# find Allegro5 game library if enabled
if(VPVL_USE_ALLEGRO5)
  find_path(ALLEG5_INCLUDE_DIRS allegro5/allegro5.h PATH $ENV{ALLEG5_INCLUDE_DIR})
  find_library(ALLEG5_LIBRARY allegro PATH $ENV{ALLEG5_LIBRARY_DIR})
  target_link_libraries(vpvl ${ALLEG5_LIBRARY})
  include_directories(${ALLEG5_INCLUDE_DIRS})
endif()

set(VPVL_OUTPUT_NAME "vpvl")
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
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
  # disable _CRT_SECURE_NO_WARNINGS for surpressing warnings from vpvl/common.h
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
else()
  install(TARGETS vpvl DESTINATION lib)
  install(DIRECTORY include/vpvl DESTINATION include PATTERN "*.h" PATTERN "internal" PATTERN ".svn" EXCLUDE)
endif()

# link with OpenGL and GLEW
if(VPVL_OPENGL_RENDERER)
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/engine/opengl vpvl_opengl_sources)
  find_package(OpenGL REQUIRED)
  target_link_libraries(vpvl ${OPENGL_LIBRARIES})
  include_directories(${OPENGL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/engine/opengl)
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
endif()

# extra renderer program with allegro5
option(VPVL_BUILD_A5 "Build a renderer program using Allegro, OpenCV and NyARToolKit to test rendering result (enabling VPVL_OPENGL_RENDERER required, default is OFF)" OFF)
if(VPVL_OPENGL_RENDERER AND VPVL_USE_ALLEGRO5 AND VPVL_BUILD_A5)
  find_package(OpenCV REQUIRED)
  find_library(ALLEG5_LIBRARY_MAIN allegro_main PATH $ENV{ALLEG5_LIBRARY_DIR})
  find_library(ALLEG5_LIBRARY_IMAGE allegro_image PATH $ENV{ALLEG5_LIBRARY_DIR})
  aux_source_directory(render/a5 vpvl_a5_sources)
  add_executable(vpvl_a5 ${vpvl_a5_sources} ${vpvl_public_headers} ${vpvl_internal_headers})
  target_link_libraries(vpvl_a5 vpvl ${ALLEG5_LIBRARY_MAIN} ${ALLEG5_LIBRARY_IMAGE} opencv_core opencv_highgui)
endif()

# extra test program
option(VPVL_BUILD_TESTS "Build test programs (default is OFF)" OFF)
if(VPVL_BUILD_TESTS)
  aux_source_directory(gtest vpvl_test_sources)
  add_executable(vpvl_test ${vpvl_test_sources} ${vpvl_public_headers} ${vpvl_internal_headers})
  find_library(GTEST_LIB gtest PATH ${GTEST_LIBRARY_DIR})
  find_library(GTEST_MAIN_LIB gtest_main PATH ${GTEST_LIBRARY_DIR})
  target_link_libraries(vpvl_test vpvl ${GTEST_MAIN_LIB} ${GTEST_LIB})
  include_directories(${GTEST_INCLUDE_DIR})
endif()
