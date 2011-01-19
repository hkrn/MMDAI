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

#
# build libGLee (OpenGL easy extension)
#

cmake_minimum_required(VERSION 2.6)

project(glee)
add_library(glee SHARED glee.c GLee.h)

# link against Foundation.framework
if(APPLE)
  find_library(FOUNDATION Foundation)
  target_link_libraries(glee ${FOUNDATION})
  set_target_properties(glee PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# find OpenGL package
find_package(OpenGL)
if(OPENGL_FOUND)
  include_directories(${OPENGL_INCLUDE_DIR})
  target_link_libraries(glee ${OPENGL_LIBRARIES})
else()
  message(FATAL_ERROR "Required OpenGL is not found.")
endif()

install(TARGETS glee LIBRARY DESTINATION lib)
install(FILES GLee.h DESTINATION include)

