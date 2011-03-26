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

include(QMACommon.pri)

QT += core gui opengl
TARGET = QtMMDAI
TEMPLATE = app

CONFIG(release, debug|release) {
    macx {
        DEFINES += QMA_BUNDLE_PLUGINS
        CONFIG += static
        # for QMAAudioPlugin
        QT += phonon
        # QMAJuliusPlugin
        JULIUS_PATHS  = /usr/local/bin /usr/bin
        for(path, JULIUS_PATHS):exists($${path}/libjulius-config):exists($${path}/libsent-config) {
            LIBS += $$system($${path}/libjulius-config --libs) $$system($${path}/libsent-config --libs)
        }
        # QMAOpenJTalkPlugin
        LIBS += -lHTSEngine -ljpcommon -lmecab2njd -lnjd -lnjd2jpcommon -lnjd_set_accent_phrase \
                -lnjd_set_accent_type -lnjd_set_digit -lnjd_set_long_vowel -lnjd_set_pronunciation \
                -lnjd_set_unvoiced_vowel -ltext2mecab -lmecab_custom -lportaudio \
                -framework CoreAudio -framework CoreFoundation -framework CoreServices \
                -framework AudioToolbox -framework AudioUnit -lportaudio
        INCLUDEPATH += /usr/local/include/jtalk
        LIBS += -L../StaticPlugins -lQMAAudioPlugin -lQMAJuliusPlugin -lQMALookAtPlugin -lQMAOpenJTalkPlugin -lQMAVIManagerPlugin -lQMAVariablePlugin
    }
}

macx:CONFIG += x86

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
    QMAModelLoader.h \
    QMAModelLoaderFactory.h \
    QMALipSyncLoder.h \
    QMALogger.h \
    QMALogViewWidget.h

TRANSLATIONS += res/translations/QMA.ts

CODECFORTR = UTF-8
