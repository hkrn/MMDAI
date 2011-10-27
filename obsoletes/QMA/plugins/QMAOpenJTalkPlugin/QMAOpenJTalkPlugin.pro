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
# /* - Neither the name of the MMDAI project team nor the names of     */
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

include(../QMAPlugin.pri)
TARGET = $$qtLibraryTarget(QMAOpenJTalkPlugin)

unix:INCLUDEPATH += /usr/local/include/OpenJTalk
unix:LIBS += -lHTSEngine -lOpenJTalk -lportaudio
macx:LIBS += -liconv -framework CoreAudio -framework CoreFoundation \
             -framework CoreServices -framework AudioToolbox -framework AudioUnit

win32 {
    # use MMDAgent's OpenJTalk and related libraries
    # located in MMDAgent/Library_hts_engine_API/lib
    INCLUDEPATH += $$(MMDAI_OPENJTALK_INCLUDE_DIR) $$(MMDAI_HTS_ENGINE_INCLUDE_DIR) $$(MMDAI_PORTAUDIO_INCLUDE_DIR)
    LIBS += -L$$(MMDAI_OPENJTALK_LIBRARY_DIR) -L$$(MMDAI_HTS_ENGINE_LIBRARY_DIR) -L$$(MMDAI_PORTAUDIO_LIBRARY_DIR) -L$$(MMDAI_DIRECTX_SDK_LIBRARY_DIR) -lOpen_JTalk -lhts_engine_API -lPortAudio -lwinmm
}

HEADERS += \
    QMAOpenJTalkPlugin.h \
    QMAOpenJTalkModel.h

SOURCES += \
    QMAOpenJTalkPlugin.cc \
    QMAOpenJTalkModel.cc
