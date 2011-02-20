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

QT += core gui opengl

TARGET = QtMMDAI
TEMPLATE = app
LIBS += -lMMDAI -lMMDME -lglee -lBulletDynamics -lBulletCollision -lBulletSoftBody -lLinearMath

unix:LIBS += -L/usr/local/lib
unix:INCLUDEPATH += /usr/include/bullet /usr/local/include/bullet
LIBS += -L$$(MMDME_LIBRARY_DIR) -L$$(MMDAI_LIBRARY_DIR) -L$$(BULLET_LIBRARY_DIR)
INCLUDEPATH += $$(MMDME_INCLUDE_DIR) $$(MMDAI_INCLUDE_DIR) $$(BULLET_INCLUDE_DIR)

CONFIG(release, debug|release) {
    #macx:CONFIG += x86 x86_64
    macx:LIBS += -lportaudio
    macx:QT += phonon

#    DEFINES += QMA_BUNDLE_PLUGINS
#    CONFIG += x86_64 static
#    JULIUS_PATHS  = /usr/local/bin /usr/bin
#    for(path, JULIUS_PATHS):exists($${path}/libjulius-config):exists($${path}/libsent-config) {
#        LIBS += $$system($${path}/libjulius-config --libs) $$system($${path}/libsent-config --libs)
#    }
#    macx:LIBS += -lHTSEngine -ljpcommon -lmecab2njd -lnjd -lnjd2jpcommon -lnjd_set_accent_phrase \
#                 -lnjd_set_accent_type -lnjd_set_digit -lnjd_set_long_vowel -lnjd_set_pronunciation \
#                 -lnjd_set_unvoiced_vowel -ltext2mecab -lmecab_custom \
#                 -framework CoreAudio -framework CoreFoundation -framework CoreServices \
#                 -framework AudioToolbox -framework AudioUnit
#    macx:INCLUDEPATH += /usr/local/include/jtalk
#    macx:LIBS += -lQMAAudioPlugin -lQMAJuliusPlugin -lQMALookAtPlugin -lQMAOpenJTalkPlugin -lQMAVIManagerPlugin
}

macx:CONFIG += x86

# unused (using framework)
#
# deploy command is macdeployqt
#
# macx {
#     QMAKE_LFLAGS += -F../Library_MMDFiles
#     LIBS = -framework MMDFiles \
#            -L/usr/local/lib -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lglee
#     INCLUDEPATH += ../Library_MMDFiles/include /usr/local/include/bullet
# }

SOURCES += main.cc\
        QMAWidget.cc \
        QMATimer.cc \
    QMAWindow.cc \
    QMAModelLoader.cc \
    QMAModelLoaderFactory.cc \
    QMALipSyncLoder.cc \
    QMALogger.cc \
    QMALogViewWidget.cc

HEADERS  += QMAWidget.h \
    QMAPlugin.h \
    QMATimer.h \
    QMAWindow.h \
    CommandDispatcher.h \
    QMAModelLoader.h \
    QMAModelLoaderFactory.h \
    QMALipSyncLoder.h \
    QMALogger.h \
    QMALogViewWidget.h

TRANSLATIONS += res/translations/QMA_ja.ts

CODECFORTR = UTF-8
