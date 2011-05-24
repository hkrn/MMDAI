cmake_minimum_required(VERSION 2.6)

# set library version
set(VPVL_VERSION 0.1.0)

# project configuration
project(libvpvl)
aux_source_directory(src vpvl_sources)
set(vpvl_public_headers
    include/vpvl/Bone.h
    include/vpvl/Constraint.h
    include/vpvl/Face.h
    include/vpvl/IK.h
    include/vpvl/IModel.h
    include/vpvl/IModelParser.h
    include/vpvl/IMotion.h
    include/vpvl/IMotionParser.h
    include/vpvl/IParser.h
    include/vpvl/Material.h
    include/vpvl/RigidBody.h
    include/vpvl/Vertex.h
    include/vpvl/common.h
    include/vpvl/vpvl.h
)
set(vpvl_internal_headers
    include/vpvl/internal/PMDModel.h
)

function(link_bullet target)
  OPTION(FIND_BULLET_BY_PKGCONFIG "Find Bullet Physics by pkg-config" OFF)
  if(FIND_BULLET_BY_PKGCONFIG)
    find_package(PkgConfig)
    pkg_search_module(BULLET REQUIRED bullet)
  endif()
  if(BULLET_FOUND)
    include_directories(${BULLET_INCLUDE_DIRS})
    target_link_libraries(${target} ${BULLET_LIBRARIES})
    link_directories(${BULLET_LIBRARY_DIRS})
  else()
    find_path(BULLET_INCLUDE_DIRS btBulletDynamicsCommon.h PATH $ENV{BULLET_INCLUDE_DIR})
    find_library(BULLET_LIB_COLLISION BulletCollision PATH $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_LIB_DYNAMICS BulletDynamics PATH $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_LIB_SOFTBODY BulletSoftBody PATH $ENV{BULLET_LIBRARY_DIR})
    find_library(BULLET_LIB_LINEAR_MATH LinearMath PATH $ENV{BULLET_LIBRARY_DIR})
    if(BULLET_INCLUDE_DIRS AND BULLET_LIB_COLLISION AND
        BULLET_LIB_DYNAMICS AND  BULLET_LIB_SOFTBODY AND
        BULLET_LIB_LINEAR_MATH)
      include_directories(${BULLET_INCLUDE_DIRS})
      target_link_libraries(
        ${target}
        ${BULLET_LIB_COLLISION}
        ${BULLET_LIB_DYNAMICS}
        ${BULLET_LIB_SOFTBODY}
        ${BULLET_LIB_LINEAR_MATH}
      )
    else()
      message(FATAL_ERROR "Required Bullet Physics is not found.")
    endif()
  endif()
endfunction()

option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
if(BUILD_SHARED_LIBS)
  set(VPVL_LIB_TYPE SHARED)
else()
  set(VPVL_LIB_TYPE STATIC)
endif()

add_library(vpvl ${VPVL_LIB_TYPE} ${vpvl_sources} ${vpvl_public_headers} ${vpvl_internal_headers})
set_target_properties(vpvl PROPERTIES VERSION ${VPVL_VERSION})
set_target_properties(vpvl PROPERTIES SO_VERSION ${VPVL_VERSION})

# project include directories
include_directories(include)

# find Bullet Physics
link_bullet(vpvl)

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

# extra renderer program
option(VPVL_BUILD_SDL "Build a renderer program using SDL to test rendering result" OFF)
if(VPVL_BUILD_SDL)
  find_package(OpenGL)
  find_package(SDL)
  find_package(SDL_image)
  aux_source_directory(render vpvl_sdl_sources)
  add_executable(vpvl_sdl ${vpvl_sdl_sources} ${vpvl_public_headers} ${vpvl_internal_headers})
  target_link_libraries(vpvl_sdl vpvl ${OPENGL_LIBRARIES} ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})
  include_directories(${OPENGL_INCLUDE_DIR} ${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})
endif()

# extra test program
option(VPVL_BUILD_TESTS "Build test programs" OFF)
if(VPVL_BUILD_TESTS)
  aux_source_directory(test vpvl_test_sources)
  add_executable(vpvl_test ${vpvl_test_sources} ${vpvl_public_headers} ${vpvl_internal_headers})
  find_library(GTEST_LIB gtest PATH ${GTEST_LIBRARY_DIR})
  find_library(GTEST_MAIN_LIB gtest_main PATH ${GTEST_LIBRARY_DIR})
  target_link_libraries(vpvl_test vpvl ${GTEST_MAIN_LIB} ${GTEST_LIB})
  include_directories(${GTEST_INCLUDE_DIR})
endif()
