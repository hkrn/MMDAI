/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "Archive.h"

#include <QtCore/QtCore>

Archive::Archive()
    : m_file(0),
      m_error(kNone),
      m_codec(0)
{
    m_codec = QTextCodec::codecForName("Shift-JIS");
}

Archive::~Archive()
{
    close();
}

void Archive::setTextCodec(QTextCodec *value)
{
    m_codec = value;
}

bool Archive::open(const QString &filename, QStringList &entries)
{
    m_file = unzOpen64(filename.toLocal8Bit().constData());
    if (m_file) {
        unz_file_info64 info;
        QByteArray filename;
        int err = unzGetGlobalInfo64(m_file, &m_header);
        if (err == UNZ_OK) {
            int nentries = m_header.number_entry;
            for (int i = 0; i < nentries; i++) {
                err = unzGetCurrentFileInfo64(m_file, &info, 0, 0, 0, 0, 0, 0);
                if (err == UNZ_OK && (info.compression_method == 0 || info.compression_method == Z_DEFLATED)) {
                    filename.resize(info.size_filename);
                    err = unzGetCurrentFileInfo64(m_file, 0, filename.data(), info.size_filename, 0, 0, 0, 0);
                    if (err == UNZ_OK) {
                        entries.append(QTextCodec::codecForName("Shift_JIS")->toUnicode(filename));
                    }
                    else {
                        m_error = kGetCurrentFileError;
                        break;
                    }
                }
                else {
                    m_error = kGetCurrentFileError;
                    break;
                }
                if (i + 1 < nentries) {
                    err = unzGoToNextFile(m_file);
                    if (err != UNZ_OK) {
                        m_error = kGoToNextFileError;
                        break;
                    }
                }
            }
            if (err == Z_OK) {
                err = unzGoToFirstFile(m_file);
                if (err != Z_OK)
                    m_error = kGoToFirstFileError;
            }
        }
        return err == Z_OK;
    }
    return false;
}

bool Archive::close()
{
    return unzClose(m_file) == Z_OK;
}

bool Archive::uncompress(const QStringList &entries)
{
    if (m_file == 0)
        return false;
    unz_file_info64 info;
    QByteArray filename, data;
    int nentries = m_header.number_entry, err = Z_OK;
    for (int i = 0; i < nentries; i++) {
        err = unzGetCurrentFileInfo64(m_file, &info, 0, 0, 0, 0, 0, 0);
        if (err == UNZ_OK && (info.compression_method == 0 || info.compression_method == Z_DEFLATED)) {
            filename.resize(info.size_filename);
            err = unzGetCurrentFileInfo64(m_file, 0, filename.data(), info.size_filename, 0, 0, 0, 0);
            if (err == UNZ_OK) {
                const QString &name = QTextCodec::codecForName("Shift_JIS")->toUnicode(filename);
                if (entries.contains(name)) {
                    data.resize(info.uncompressed_size);
                    err = unzOpenCurrentFile(m_file);
                    if (err != Z_OK) {
                        m_error = kOpenCurrentFileError;
                        break;
                    }
                    err = unzReadCurrentFile(m_file, data.data(), info.uncompressed_size);
                    if (err < 0) {
                        m_error = kReadCurrentFileError;
                        break;
                    }
                    err = unzCloseCurrentFile(m_file);
                    if (err != Z_OK) {
                        m_error = kCloseCurrentFileError;
                        break;
                    }
                    m_entries.insert(name, data);
                }
            }
            else {
                m_error = kGetCurrentFileError;
                break;
            }
        }
        else {
            m_error = kGetCurrentFileError;
            break;
        }
        if (i + 1 < nentries) {
            err = unzGoToNextFile(m_file);
            if (err != UNZ_OK) {
                m_error = kGoToNextFileError;
                break;
            }
        }
    }
    if (err == Z_OK) {
        err = unzGoToFirstFile(m_file);
        if (err != Z_OK)
            m_error = kGoToFirstFileError;
    }
    return err == Z_OK;
}

void Archive::replaceFilePath(const QString &from, const QString &to)
{
    QHash<QString, QByteArray> newEntries;
    QHashIterator<QString, QByteArray> it(m_entries);
    const QRegExp regexp("^" + from + "/");
    while (it.hasNext()) {
        it.next();
        QString key = it.key();
        const QByteArray &bytes = it.value();
        /* 一致した場合はパスを置換するが、ディレクトリ名が入っていないケースで一致しない場合はパスを追加 */
        if (regexp.indexIn(key) != -1) {
            key.replace(regexp, to);
            newEntries.insert(key, bytes);
        }
        else {
            key = to + key;
            newEntries.insert(key, bytes);
        }
    }
    m_entries = newEntries;
}

Archive::ErrorType Archive::error() const
{
    return m_error;
}

const QStringList Archive::entryNames() const
{
    return m_entries.keys();
}

const QByteArray Archive::data(const QString &name) const
{
    return m_entries[name];
}
