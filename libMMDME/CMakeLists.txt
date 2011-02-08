# /* ----------------------------------------------------------------- */
# /*                                                                   */
# /*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
# /*                                                                   */
# /* All rights reserved.                                              */
# /*                                                                   */
# /* Redistribution and use in source and binary forms, with or        */
# /* without modification, are permitted provided that the following   */
# /* conditions are met:                                               */
# /*                                                                   */
# /* - Redistributions of source code must retain the above copyright  */
# /*   notice, this list of conditions and the following disclaimer.   */
# /* - Redistributions in binary form must reproduce the above         */
# /*   copyright notice, this list of conditions and the following     */
# /*   disclaimer in the documentation and/or other materials provided */
# /*   with the distribution.                                          */
# /* - Neither the name of the MMDAgent project team nor the names of  */
# /*   its contributors may be used to endorse or promote products     */
# /*   derived from this software without specific prior written       */
# /*   permission.                                                     */
# /*                                                                   */
# /* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
# /* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
# /* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
# /* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
# /* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
# /* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
# /* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
# /* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
# /* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
# /* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
# /* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
# /* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
# /* POSSIBILITY OF SUCH DAMAGE.                                       */
# /* ----------------------------------------------------------------- */

cmake_minimum_required(VERSION 2.6)

# set library version
set(MMDME_VERSION 1.0)

# project configuration
project(libMMDME)
aux_source_directory(src libMMDME_sources)
set(libMMDME_public_headers
  include/MMDME/BulletPhysics.h
  include/MMDME/MotionController.h
  include/MMDME/MotionManager.h
  include/MMDME/MMDME.h
  include/MMDME/PMDBone.h
  include/MMDME/PMDConstraint.h
  include/MMDME/PMDFace.h
  include/MMDME/PMDFile.h
  include/MMDME/PMDIK.h
  include/MMDME/PMDMaterial.h 
  include/MMDME/PMDModel.h 
  include/MMDME/PMDModelLoader.h 
  include/MMDME/PMDRigidBody.h 
  include/MMDME/PMDTexture.h 
  include/MMDME/PTree.h 
  include/MMDME/VMD.h 
  include/MMDME/VMDFile.h
)

option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
if(BUILD_SHARED_LIBS)
  set(MMDME_LIB_TYPE SHARED)
else()
  set(MMDME_LIB_TYPE STATIC)
endif()

add_library(MMDME ${MMDME_LIB_TYPE} ${libMMDME_sources} ${libMMDME_public_headers})
set_target_properties(MMDME PROPERTIES VERSION ${MMDME_VERSION})
set_target_properties(MMDME PROPERTIES SO_VERSION ${MMDME_VERSION})

# project include directories
include_directories(include)

# find OpenGL package
find_package(OpenGL)
if(OPENGL_FOUND)
  include_directories(${OPENGL_INCLUDE_DIR})
  target_link_libraries(MMDME ${OPENGL_LIBRARIES})
else()
  message(FATAL_ERROR "Required OpenGL is not found.")
endif()

# find OpenGL Easy Extension
find_path(GLEE_INCLUDE GLee.h PATH $ENV{MMDAI_GLEE_DIR})
find_library(GLEE_LIBRARY glee PATH $ENV{MMDAI_GLEE_DIR})
if(GLEE_INCLUDE AND GLEE_LIBRARY)
  target_link_libraries(MMDME ${GLEE_LIBRARY})
  include_directories(${GLEE_INCLUDE})
else()
  message(FATAL_ERROR "Required OpenGL Easy Extension is not found.")
endif()

# find Bullet Physics
OPTION(FIND_BULLET_BY_PKGCONFIG "Find Bullet Physics by pkg-config" OFF)
if (FIND_BULLET_BY_PKGCONFIG)
  find_package(PkgConfig)
  pkg_search_module(BULLET REQUIRED bullet)
endif()
if(BULLET_FOUND)
  include_directories(${BULLET_INCLUDE_DIRS})
  target_link_libraries(MMDME ${BULLET_LIBRARIES})
  link_directories(${BULLET_LIBRARY_DIRS})
else()
  find_path(BULLET_INCLUDE_DIRS btBulletDynamicsCommon.h PATH $ENV{MMDAI_BULLET_INCLUDE_DIR})
  find_library(BULLET_LIB_COLLISION BulletCollision PATH $ENV{MMDAI_BULLET_LIB_DIR})
  find_library(BULLET_LIB_DYNAMICS BulletDynamics PATH $ENV{MMDAI_BULLET_LIB_DIR})
  find_library(BULLET_LIB_LINEAR_MATH LinearMath PATH $ENV{MMDAI_BULLET_LIB_DIR})
  if (BULLET_INCLUDE_DIRS AND BULLET_LIB_COLLISION AND
	  BULLET_LIB_DYNAMICS AND BULLET_LIB_LINEAR_MATH)
    include_directories(${BULLET_INCLUDE_DIRS})
    target_link_libraries(MMDME
      ${BULLET_LIB_COLLISION}
      ${BULLET_LIB_DYNAMICS}
      ${BULLET_LIB_LINEAR_MATH}
    )
  else()
    message(FATAL_ERROR "Required Bullet Physics is not found.")
  endif()
endif()

# create as a framework if build on darwin environment
if(APPLE)
  if(BUILD_SHARED_LIBS AND FRAMEWORK)
    install(TARGETS MMDME DESTINATION .)
    set_target_properties(MMDME PROPERTIES FRAMEWORK true)
    set_target_properties(MMDME PROPERTIES PUBLIC_HEADER "${libMMDME_public_headers}")
  endif()
  set_target_properties(MMDME PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

if(NOT MSVC)
  install(TARGETS MMDME DESTINATION lib)
  install(DIRECTORY include/MMDME DESTINATION include PATTERN "*.h" PATTERN ".svn" EXCLUDE)
endif()

