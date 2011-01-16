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
set(MMDFILES_VERSION 1.0)

# project configuration
project(Library_MMDFiles)
aux_source_directory(src libMMDFiles_sources)
set(libMMDFiles_public_headers
  include/BulletPhysics.h
  include/MotionController.h
  include/MotionManager.h
  include/MMDFiles.h
  include/PMDBone.h
  include/PMDConstraint.h
  include/PMDFace.h
  include/PMDFile.h
  include/PMDIK.h
  include/PMDMaterial.h 
  include/PMDModel.h 
  include/PMDRigidBody.h 
  include/PMDTexture.h 
  include/PMDTextureLoader.h 
  include/PTree.h 
  include/SystemTexture.h 
  include/VMD.h 
  include/VMDFile.h
)

add_library(MMDFiles SHARED ${libMMDFiles_sources} ${libMMDFiles_public_headers})
set_target_properties(MMDFiles PROPERTIES VERSION ${MMDFILES_VERSION})
set_target_properties(MMDFiles PROPERTIES SO_VERSION ${MMDFILES_VERSION})

# project include directories
include_directories(include)

# find zlib package
find_package(ZLIB)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIR})
  target_link_libraries(MMDFiles ${ZLIB_LIBRARIES})
else()
  message(FATAL_ERROR "Required zlib is not found.")
endif()

# find libpng package
find_package(PNG)
if(PNG_FOUND)
  include_directories(${PNG_INCLUDE_DIR})
  add_definitions(${PNG_DEFINITIONS})
  target_link_libraries(MMDFiles ${PNG_LIBRARIES})
else()
  message(FATAL_ERROR "Required libpng is not found.")
endif()

# find OpenGL package
find_package(OpenGL)
if(OPENGL_FOUND)
  include_directories(${OPENGL_INCLUDE_DIR})
  target_link_libraries(MMDFiles ${OPENGL_LIBRARIES})
else()
  message(FATAL_ERROR "Required OpenGL is not found.")
endif()

# find OpenGL Easy Extension
find_path(GLEE_INCLUDE GLee.h /usr/local/include /usr/include)
find_library(GLEE_LIBRARY glee /usr/local/lib /usr/lib)
if(GLEE_INCLUDE AND GLEE_LIBRARY)
  target_link_libraries(MMDFiles ${GLEE_LIBRARY})
else()
  message(FATAL_ERROR "Required OpenGL Easy Extension is not found.")
endif()

# find Bullet Physics via pkg-config
find_package(PkgConfig)
pkg_search_module(BULLET REQUIRED bullet)
if(BULLET_FOUND)
  include_directories(${BULLET_INCLUDE_DIRS})
  target_link_libraries(MMDFiles ${BULLET_LIBRARIES})
  link_directories(${BULLET_LIBRARY_DIRS})
else()
  message(FATAL_ERROR "Required Bullet Physics is not found.")
endif()

# create as a framework if build on darwin environment
if(APPLE)
  if(BUILD_SHARED_LIBS AND FRAMEWORK)
    install(TARGETS MMDFiles DESTINATION .)
    set_target_properties(MMDFiles PROPERTIES FRAMEWORK true)
    set_target_properties(MMDFiles PROPERTIES PUBLIC_HEADER "${libMMDFiles_public_headers}")
  endif()
  set_target_properties(MMDFiles PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

install(TARGETS MMDFiles EXPORT MMDFiles LIBRARY DESTINATION lib)
install(EXPORT MMDFiles DESTINATION lib/MMDFiles)
install(DIRECTORY include/ DESTINATION include/MMDFiles PATTERN "*.h" PATTERN ".svn" EXCLUDE)

