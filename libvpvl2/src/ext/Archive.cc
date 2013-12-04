/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/Archive.h>
#include <vpvl2/extensions/icu4c/String.h>
#include <vpvl2/internal/util.h>

#include <algorithm>
#include <map>
#include <unicode/regex.h>

#include "ioapi.h"
#include "unzip.h"

namespace vpvl2
{
namespace extensions
{
using namespace icu4c;

struct Archive::PrivateContext {
    PrivateContext(IEncoding *encodingRef)
        : file(0),
          error(kNone),
          encodingRef(encodingRef)
    {
    }
    ~PrivateContext() {
        close();
    }

    bool close() {
        originalEntries.clear();
        unicodePath2RawPath.clear();
        int ret = unzClose(file);
        file = 0;
        return ret == Z_OK;
    }
    bool uncompressEntry(const std::string &entry, const unz_file_info &finfo) {
        std::string &bytes = originalEntries[entry];
        uint32 size(finfo.uncompressed_size);
        bytes.resize(size);
        VPVL2_VLOG(1, "filename=" << entry << " size=" << size);
        int err = unzOpenCurrentFile(file);
        if (err != Z_OK) {
            VPVL2_LOG(WARNING, "Cannot open the file " << entry << " in zip: " << err);
            error = kOpenCurrentFileError;
            return false;
        }
        err = unzReadCurrentFile(file, &bytes[0], size);
        if (err < 0) {
            VPVL2_LOG(WARNING, "Cannot read the file " << entry << " in zip: " << err);
            error = kReadCurrentFileError;
            return false;
        }
        err = unzCloseCurrentFile(file);
        if (err != Z_OK) {
            VPVL2_LOG(WARNING, "Cannot close the file " << entry << " in zip: " << err);
            error = kCloseCurrentFileError;
            return false;
        }
        return true;
    }
    std::string resolvePath(const std::string &value) const {
        return basePath.empty() ? value : basePath + "/" + value;
    }

    typedef std::map<std::string, std::string> EntryDataMap;
    typedef std::map<std::string, std::string> UnicodePath2RawPathMap;
    unzFile file;
    unz_global_info header;
    Archive::ErrorType error;
    const IEncoding *encodingRef;
    EntryDataMap originalEntries;
    UnicodePath2RawPathMap unicodePath2RawPath;
    std::string basePath;
};

Archive::Archive(IEncoding *encodingRef)
    : m_context(new PrivateContext(encodingRef))
{
}

Archive::~Archive()
{
    internal::deleteObject(m_context);
}

bool Archive::open(const IString *filename, EntryNames &entries)
{
    m_context->file = unzOpen(reinterpret_cast<const char *>(filename->toByteArray()));
    if (m_context->file) {
        unz_file_info info;
        std::string path;
        int err = unzGetGlobalInfo(m_context->file, &m_context->header);
        if (err == UNZ_OK) {
            uLong nentries = m_context->header.number_entry;
            for (uLong i = 0; i < nentries; i++) {
                err = unzGetCurrentFileInfo(m_context->file, &info, 0, 0, 0, 0, 0, 0);
                if (err == UNZ_OK && (info.compression_method == 0 || info.compression_method == Z_DEFLATED)) {
                    path.resize(info.size_filename);
                    err = unzGetCurrentFileInfo(m_context->file, &info, &path[0], info.size_filename, 0, 0, 0, 0);
                    if (err == UNZ_OK) {
                        const uint8 *ptr = reinterpret_cast<const uint8 *>(path.data());
                        IString *s = m_context->encodingRef->toString(ptr, path.size(), IString::kShiftJIS);
                        const std::string &value = String::toStdString(static_cast<const String *>(s)->value());
                        entries.push_back(value);
                        m_context->unicodePath2RawPath.insert(std::make_pair(value, path));
                        internal::deleteObject(s);
                    }
                    else {
                        VPVL2_LOG(WARNING, "Cannot get current file " << path << " in zip: " << err);
                        m_context->error = kGetCurrentFileError;
                        break;
                    }
                }
                else {
                    VPVL2_LOG(WARNING, "Cannot get current file " << path << " in zip: " << err);
                    m_context->error = kGetCurrentFileError;
                    break;
                }
                if (i + 1 < nentries) {
                    err = unzGoToNextFile(m_context->file);
                    if (err != UNZ_OK) {
                        VPVL2_LOG(WARNING, "Cannot seek next current file from " << path << " in zip: " << err);
                        m_context->error = kGoToNextFileError;
                        break;
                    }
                }
            }
            if (err == Z_OK) {
                err = unzGoToFirstFile(m_context->file);
                if (err != Z_OK) {
                    VPVL2_LOG(WARNING, "Cannot seek to the first file in zip: " << err);
                    m_context->error = kGoToFirstFileError;
                }
            }
        }
        return err == Z_OK;
    }
    return false;
}

bool Archive::close()
{
    return m_context->close();
}

bool Archive::uncompress(const EntrySet &entries)
{
    if (m_context->file == 0) {
        return false;
    }
    unz_file_info info;
    std::string filename, entry;
    uLong nentries = m_context->header.number_entry;
    int err = Z_OK;
    for (uLong i = 0; i < nentries; i++) {
        err = unzGetCurrentFileInfo(m_context->file, &info, 0, 0, 0, 0, 0, 0);
        if (err == UNZ_OK && (info.compression_method == 0 || info.compression_method == Z_DEFLATED)) {
            filename.resize(info.size_filename);
            err = unzGetCurrentFileInfo(m_context->file, &info, &filename[0], info.size_filename, 0, 0, 0, 0);
            if (err == UNZ_OK) {
                const uint8 *ptr = reinterpret_cast<const uint8 *>(filename.data());
                if (IString *name = m_context->encodingRef->toString(ptr, filename.size(), IString::kShiftJIS)) {
                    /* normalize filename with lower */
                    entry.assign(String::toStdString(static_cast<const String *>(name)->value().toLower()));
                    internal::deleteObject(name);
                }
                if (entries.find(entry) != entries.end() && !m_context->uncompressEntry(entry, info)) {
                    return false;
                }
            }
            else {
                m_context->error = kGetCurrentFileError;
                break;
            }
        }
        else {
            m_context->error = kGetCurrentFileError;
            break;
        }
        if (i + 1 < nentries) {
            err = unzGoToNextFile(m_context->file);
            if (err != UNZ_OK) {
                VPVL2_LOG(WARNING, "Cannot seek next current file in zip: " << err);
                m_context->error = kGoToNextFileError;
                break;
            }
        }
    }
    if (err == Z_OK) {
        err = unzGoToFirstFile(m_context->file);
        if (err != Z_OK) {
            VPVL2_LOG(WARNING, "Cannot seek to the first file in zip: " << err);
            m_context->error = kGoToFirstFileError;
        }
    }
    return err == Z_OK;
}

bool Archive::uncompressEntry(const std::string &name)
{
    const std::string &key = m_context->resolvePath(name);
    PrivateContext::UnicodePath2RawPathMap::const_iterator it = m_context->unicodePath2RawPath.find(key);
    bool ok = false;
    if (it != m_context->unicodePath2RawPath.end()) {
        int err = unzLocateFile(m_context->file, it->second.c_str(), 1);
        if (err == Z_OK) {
            unz_file_info finfo;
            unzGetCurrentFileInfo(m_context->file, &finfo, 0, 0, 0, 0, 0, 0);
            ok = m_context->uncompressEntry(it->first, finfo);
            unzGoToFirstFile(m_context->file);
        }
        else {
            VPVL2_LOG(WARNING, "Cannot locate to the file << " << name << " in zip: " << err);
        }
    }
    return ok;
}

void Archive::setBasePath(const std::string &value)
{
    m_context->basePath = value;
}

Archive::ErrorType Archive::error() const
{
    return m_context->error;
}

const Archive::EntryNames Archive::entryNames() const
{
    PrivateContext::EntryDataMap::const_iterator it = m_context->originalEntries.begin();
    EntryNames names;
    while (it != m_context->originalEntries.end()) {
        names.push_back(it->first);
        ++it;
    }
    return names;
}

const std::string *Archive::dataRef(const std::string &name) const
{
    std::string ln = name;
    std::transform(ln.begin(), ln.end(), ln.begin(), ::tolower);
    PrivateContext::EntryDataMap::const_iterator it = m_context->originalEntries.find(m_context->resolvePath(ln));
    return it != m_context->originalEntries.end() ? &it->second : 0;
}

} /* namespace extensions */
} /* namespace vpvl2 */
