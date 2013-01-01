/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#pragma once
#ifndef VPVL2_PROJECT_H_
#define VPVL2_PROJECT_H_

#include <string>
#include <vector>

#include "vpvl2/Scene.h"

/* declare libxml's handle ahead */
typedef struct _xmlTextWriter* xmlTextWriterPtr;
typedef struct _xmlBuffer* xmlBufferPtr;

namespace vpvl2
{

class IString;
class Factory;

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Project class represents a project file (*.vpvx)
 */

class VPVL2_API Project : public Scene
{
public:
    class IDelegate
    {
    public:
        virtual ~IDelegate() {}
        virtual const std::string toStdFromString(const IString *value) const = 0;
        virtual const IString *toStringFromStd(const std::string &value) const = 0;
        virtual void error(const char *format, va_list ap) = 0;
        virtual void warning(const char *format, va_list ap) = 0;
    };
    typedef std::string UUID;
    typedef std::vector<UUID> UUIDList;

    static const UUID kNullUUID;
    static const std::string kSettingNameKey;
    static const std::string kSettingURIKey;
    static const std::string kSettingArchiveURIKey;

    static float formatVersion();
    static bool isReservedSettingKey(const std::string &key);

    Project(IDelegate *delegate, Factory *factory, bool ownMemory);
    ~Project();

    bool load(const char *path);
    bool load(const uint8_t *data, size_t size);
    bool save(const char *path);
    bool save(xmlBufferPtr &buffer);

    std::string version() const;
    std::string globalSetting(const std::string &key) const;
    std::string modelSetting(const IModel *model, const std::string &key) const;
    const UUIDList modelUUIDs() const;
    const UUIDList motionUUIDs() const;
    UUID modelUUID(const IModel *model) const;
    UUID motionUUID(const IMotion *motion) const;
    IModel *findModel(const UUID &uuid) const;
    IMotion *findMotion(const UUID &uuid) const;
    bool containsModel(const IModel *model) const;
    bool containsMotion(const IMotion *motion) const;
    bool isDirty() const;
    void setDirty(bool value);

    void addModel(IModel *model, IRenderEngine *engine, const UUID &uuid);
    void addMotion(IMotion *motion, const UUID &uuid);
    void removeModel(IModel *model);
    void removeMotion(IMotion *motion);

    void setGlobalSetting(const std::string &key, const std::string &value);
    void setModelSetting(const IModel *model, const std::string &key, const std::string &value);

private:
    struct PrivateContext;
    bool save0(xmlTextWriterPtr ptr);
    bool validate(bool result);

    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Project)
};

} /* namespace vpvl */

#endif
