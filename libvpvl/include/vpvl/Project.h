/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#ifndef VPVL_PROJECT_H_
#define VPVL_PROJECT_H_

#include <libxml2/libxml/SAX2.h>
#include <string>
#include <vector>

#include "vpvl/Common.h"

typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace vpvl
{

class Asset;
class PMDModel;
class VMDMotion;

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Project class represents a project file (*.vpvx)
 */

class VPVL_API Project
{
public:
    class IDelegate
    {
    public:
        virtual const std::string toUnicode(const std::string &value) = 0;
        virtual void error(const char *format, va_list ap) = 0;
        virtual void warning(const char *format, va_list ap) = 0;
    };
    typedef struct Handler Handler;
    typedef std::vector<std::string> UUIDList;

    static const float kCurrentVersion;
    static const std::string kSettingNameKey;
    static const std::string kSettingURIKey;

    static bool isReservedSettingKey(const std::string &key);

    Project(IDelegate *delegate);
    ~Project();

    bool load(const char *path);
    bool load(const uint8_t *data, size_t size);
    bool save(const char *path);
    bool save(xmlBufferPtr &buffer);

    float version() const;
    bool isPhysicsEnabled() const;
    const std::string &globalSetting(const std::string &key) const;
    const std::string &assetSetting(Asset *asset, const std::string &key) const;
    const std::string &modelSetting(PMDModel *model, const std::string &key) const;
    const UUIDList assetUUIDs() const;
    const UUIDList modelUUIDs() const;
    const UUIDList motionUUIDs() const;
    Asset *asset(const std::string &uuid) const;
    PMDModel *model(const std::string &uuid) const;
    VMDMotion *motion(const std::string &uuid) const;
    bool containsAsset(Asset *asset) const;
    bool containsModel(PMDModel *model) const;
    bool containsMotion(VMDMotion *motion) const;
    bool isDirty() const { return m_dirty; }
    void setDirty(bool value) { m_dirty = value; }

    void addAsset(Asset *asset, const std::string &uuid);
    void addModel(PMDModel *model, const std::string &uuid);
    void addMotion(VMDMotion *motion, PMDModel *model, const std::string &uuid);
    void deleteAsset(Asset *&asset);
    void deleteModel(PMDModel *&model);
    void deleteMotion(VMDMotion *&motion, PMDModel *model);
    void removeAsset(Asset *asset);
    void removeModel(PMDModel *model);
    void removeMotion(VMDMotion *motion, PMDModel *model);

    void setPhysicsEnable(bool value);
    void setGlobalSetting(const std::string &key, std::string &value);
    void setAssetSetting(Asset *asset, const std::string &key, const std::string &value);
    void setModelSetting(PMDModel *model, const std::string &key, const std::string &value);

private:
    bool save0(xmlTextWriterPtr ptr);

    Handler *m_handler;
    xmlSAXHandler m_sax;
    bool m_dirty;

    VPVL_DISABLE_COPY_AND_ASSIGN(Project)
};

} /* namespace vpvl */

#endif
