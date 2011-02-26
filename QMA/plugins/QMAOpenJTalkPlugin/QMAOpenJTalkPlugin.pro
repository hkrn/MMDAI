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
QT += phonon
TARGET = $$qtLibraryTarget(QMAOpenJTalkPlugin)

# $ cd /usr/local/include
# $ sudo mkdir jtabbbk
# $ cd jtalk
# $ sudo find $OPEN_JTALK_SRC_DIR -name '*.h' -exec ln -s {} \;
# $ cd /usr/local/lib
# $ sudo find $OPEN_JTALK_SRC_DIR -name '*.a' -exec ln -s {} \;
unix:LIBS += -lHTSEngine -ljpcommon -lmecab2njd -lnjd -lnjd2jpcommon -lnjd_set_accent_phrase \
             -lnjd_set_accent_type -lnjd_set_digit -lnjd_set_long_vowel -lnjd_set_pronunciation \
             -lnjd_set_unvoiced_vowel -ltext2mecab
unix:INCLUDEPATH += /usr/local/include/jtalk

linux-g++:LIBS += -lmecab


# $ cd /usr/local/include
# $ sudo mkdir jtalk
# $ cd jtalk
# $ sudo gfind $OPEN_JTALK_SRC_DIR -name '*.h' -exec ln -s {} \;
# $ cd /usr/local/lib
# $ sudo gfind $OPEN_JTALK_SRC_DIR -name '*.a' -exec ln -s {} \;
# $ sudo mv libmecab.a libmecab_custom.a
#
# on MacOSX, mecab has been installed in /usr, we use jtalk's mecab as libmecab_custom.a
macx:LIBS += -liconv -lmecab_custom
macx:CONFIG += x86

win32 {
    # use MMDAgent's OpenJTalk and related libraries
    # located in MMDAgent/Library_hts_engine_API/lib
    LIBS:debug += -lOpenJTalk_D -lhts_engine_API_D -lwinmm
    LIBS:release += -lOpenJTalk -lhts_engine_API -lwinmm
}

HEADERS += \
    QMAOpenJTalkPlugin.h \
    QMAOpenJTalkModel.h

SOURCES += \
    QMAOpenJTalkPlugin.cc \
    QMAOpenJTalkModel.cc
