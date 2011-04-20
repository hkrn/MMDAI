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
set(MMDAI_VERSION 1.0)

# project configuration
project(libMMDAI)
aux_source_directory(src libMMDAI_sources)
set(libMMDAI_public_headers
  include/MMDAI/BoneController.h
  include/MMDAI/CommandParser.h
  include/MMDAI/ILipSyncLoader.h
  include/MMDAI/IResourceFactory.h
  include/MMDAI/LipSync.h
  include/MMDAI/MotionStocker.h
  include/MMDAI/MMDAI.h
  include/MMDAI/PMDObject.h
  include/MMDAI/Preference.h
  include/MMDAI/SceneController.h
  include/MMDAI/SceneEventHandler.h
  include/MMDAI/SceneRenderEngine.h
  include/MMDAI/Stage.h
  include/MMDAI/TileTexture.h
)

option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
if(BUILD_SHARED_LIBS)
  set(MMDAI_LIB_TYPE SHARED)
else()
  set(MMDAI_LIB_TYPE STATIC)
endif()

add_library(MMDAI ${MMDAI_LIB_TYPE} ${libMMDAI_sources} ${libMMDAI_public_headers})
set_target_properties(MMDAI PROPERTIES VERSION ${MMDAI_VERSION})
set_target_properties(MMDAI PROPERTIES SO_VERSION ${MMDAI_VERSION})

# project include directories
include_directories(include)

# find OpenGL package
find_package(OpenGL)
if(OPENGL_FOUND)
  include_directories(${OPENGL_INCLUDE_DIR})
  target_link_libraries(MMDAI ${OPENGL_LIBRARIES})
else()
  message(FATAL_ERROR "Required OpenGL is not found.")
endif()

# find OpenGL Easy Extension
if(NOT APPLE)
  if (MSVC)
    # disable _CRT_SECURE_NO_WARNINGS for surpressing warnings from MMDME/Common.h
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
  endif()
  find_path(GLEE_INCLUDE GLee.h PATH $ENV{GLEE_DIR})
  find_library(GLEE_LIBRARY glee PATH $ENV{GLEE_DIR})
  if(GLEE_INCLUDE AND GLEE_LIBRARY)
    target_link_libraries(MMDAI ${GLEE_LIBRARY})
    include_directories(${GLEE_INCLUDE})
  else()
    message(FATAL_ERROR "Required OpenGL Easy Extension is not found.")
  endif()
endif()

# find Bullet Physics
option(FIND_BULLET_BY_PKGCONFIG "Find Bullet Physics by pkg-config" OFF)
if (FIND_BULLET_BY_PKGCONFIG)
  find_package(PkgConfig)
  pkg_search_module(BULLET REQUIRED bullet)
endif()
if(BULLET_FOUND)
  include_directories(${BULLET_INCLUDE_DIRS})
  target_link_libraries(MMDAI ${BULLET_LIBRARIES})
  link_directories(${BULLET_LIBRARY_DIRS})
else()
  find_path(BULLET_INCLUDE_DIRS btBulletDynamicsCommon.h PATH $ENV{BULLET_INCLUDE_DIR})
  find_library(BULLET_LIB_COLLISION BulletCollision PATH $ENV{BULLET_LIBRARY_DIR})
  find_library(BULLET_LIB_DYNAMICS BulletDynamics PATH $ENV{BULLET_LIBRARY_DIR})
  find_library(BULLET_LIB_SOFTBODY BulletSoftBody PATH $ENV{BULLET_LIBRARY_DIR})
  find_library(BULLET_LIB_LINEAR_MATH LinearMath PATH $ENV{BULLET_LIBRARY_DIR})
  if (BULLET_INCLUDE_DIRS AND BULLET_LIB_COLLISION AND
      BULLET_LIB_DYNAMICS AND  BULLET_LIB_SOFTBODY AND
      BULLET_LIB_LINEAR_MATH)
    include_directories(${BULLET_INCLUDE_DIRS})
    target_link_libraries(MMDAI
      ${BULLET_LIB_COLLISION}
      ${BULLET_LIB_DYNAMICS}
      ${BULLET_LIB_SOFTBODY}
      ${BULLET_LIB_LINEAR_MATH}
    )
  else()
    message(FATAL_ERROR "Required Bullet Physics is not found.")
  endif()
endif()

# find libMMDME
find_path(MMDME_INCLUDE MMDME/MMDME.h PATH $ENV{MMDME_INCLUDE_DIR})
find_library(MMDME_LIBRARY MMDME PATH $ENV{MMDME_LIBRARY_DIR})
if(MMDME_INCLUDE AND MMDME_LIBRARY)
  target_link_libraries(MMDAI ${MMDME_LIBRARY})
  include_directories(MMDAI ${MMDME_INCLUDE})
else()
  message(FATAL_ERROR "Required MMDME is not found.")
endif()

option(USE_OPENGL_ES1 "Use OpenGLES 1.x instead of desktop OpenGL" OFF)
if (USE_OPENGL_ES1)
  add_definitions(-DOPENGLES1)
endif()

# create as a framework if build on darwin environment
if(APPLE)
  if(BUILD_SHARED_LIBS AND FRAMEWORK)
    install(TARGETS MMDAI DESTINATION .)
    set_target_properties(MMDAI PROPERTIES FRAMEWORK true)
    set_target_properties(MMDAI PROPERTIES PUBLIC_HEADER "${libMMDAI_public_headers}")
  endif()
  set_target_properties(MMDAI PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

if(NOT MSVC)
  install(TARGETS MMDAI DESTINATION lib)
  install(DIRECTORY include/MMDAI DESTINATION include PATTERN "*.h" PATTERN ".svn" EXCLUDE)
endif()

