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
set(MMDAGENT_VERSION 1.0)

# project configuration
project(libMMDAI)
aux_source_directory(src libMMDAI_sources)
set(libMMDAI_public_headers
  include/BoneController.h
  include/CommandParser.h
  include/LipSync.h
  include/Logger.h
  include/MotionStocker.h
  include/Option.h
  include/PMDModelLoaderFactory.h
  include/PMDObject.h
  include/SceneController.h
  include/SceneEventHandler.h
  include/SceneRenderer.h
  include/Stage.h
  include/TextRenderer.h
  include/TileTexture.h
)

add_library(MMDAI SHARED ${libMMDAI_sources} ${libMMDAI_public_headers})
set_target_properties(MMDAI PROPERTIES VERSION ${MMDAGENT_VERSION})
set_target_properties(MMDAI PROPERTIES SO_VERSION ${MMDAGENT_VERSION})

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
find_path(GLEE_INCLUDE GLee.h /usr/local/include /usr/include)
find_library(GLEE_LIBRARY glee /usr/local/lib /usr/lib)
if(GLEE_INCLUDE AND GLEE_LIBRARY)
  target_link_libraries(MMDAI ${GLEE_LIBRARY})
  include_directories(MMDAI ${GLEE_INCLUDE})
else()
  message(FATAL_ERROR "Required OpenGL Easy Extension is not found.")
endif()

if (MINGW)
  # find Bullet Physics
  find_path(BULLET_INCLUDE_DIR btBulletDynamicsCommon.h)
  find_library(BULLET_LIB_COLLISION BulletCollision)
  find_library(BULLET_LIB_DYNAMICS BulletDynamics)
  find_library(BULLET_LIB_SOFT_BODY BulletSoftBody)
  find_library(BULLET_LIB_LINEAR_MATH LinearMath)
  if (BULLET_INCLUDE_DIR)
    include_directories(${BULLET_INCLUDE_DIRS})
    target_link_libraries(MMDAI
      ${BULLET_LIB_COLLISION}
      ${BULLET_LIB_DYNAMICS}
      ${BULLET_LIB_SOFT_BODY}
      ${BULLET_LIB_LINEAR_MATH}
    )
  else()
    message(FATAL_ERROR "Required Bullet Physics is not found.")
  endif()
else()
  # find Bullet Physics via pkg-config
  find_package(PkgConfig)
  pkg_search_module(BULLET REQUIRED bullet)
  if(BULLET_FOUND)
    include_directories(${BULLET_INCLUDE_DIRS})
    target_link_libraries(MMDAI ${BULLET_LIBRARIES})
    link_directories(${BULLET_LIBRARY_DIRS})
  else()
    message(FATAL_ERROR "Required Bullet Physics is not found.")
  endif()
endif()

# find libMMDFiles
find_path(MMDFILES_INCLUDE MMDFiles/PMDModel.h)
find_library(MMDFILES_LIBRARY MMDFiles)
if(MMDFILES_INCLUDE AND MMDFILES_LIBRARY)
  target_link_libraries(MMDAI ${MMDFILES_LIBRARY})
  include_directories(MMDAI ${MMDFILES_INCLUDE})
else()
  message(FATAL_ERROR "Required MMDFiles is not found.")
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

install(TARGETS MMDAI DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/MMDAI PATTERN "*.h" PATTERN ".svn" EXCLUDE)

