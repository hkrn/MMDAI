/**

 Copyright (c) 2010-2014  hkrn

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

#pragma once
#ifndef VPVL2_EXTENSIONS_XMLPROJECT_H_
#define VPVL2_EXTENSIONS_XMLPROJECT_H_

#include <string>
#include <vector>
#include <map>

#include <vpvl2/IModel.h>
#include <vpvl2/Scene.h>
#include <vpvl2/extensions/StringMap.h>

namespace vpvl2
{
class IString;
class Factory;

namespace extensions
{

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Project class represents a project file (*.vpvx)
 */

class VPVL2_API XMLProject VPVL2_DECL_FINAL : public Scene
{
public:
    typedef std::string UUID;
    typedef std::vector<UUID> UUIDList;
    typedef std::map<XMLProject::UUID, StringMap> ModelSettings;

    class IDelegate {
    public:
        virtual ~IDelegate() {}
        virtual const std::string toStdFromString(const IString *value) const = 0;
        virtual const IString *toStringFromStd(const std::string &value) const = 0;
        virtual bool loadModel(const UUID &uuid, const StringMap &settings, IModel::Type type, IModel *&model, IRenderEngine *&engine, int &priority) = 0;
    };

    static const UUID kNullUUID;
    static const std::string kSettingNameKey;
    static const std::string kSettingURIKey;
    static const std::string kSettingArchiveURIKey;
    static const std::string kSettingOrderKey;

    static float32 formatVersion();
    static bool isReservedSettingKey(const std::string &key);
    static std::string toStringFromFloat32(float32 value);
    static std::string toStringFromVector3(const Vector3 &value);
    static std::string toStringFromVector4(const Vector4 &value);
    static std::string toStringFromQuaternion(const Quaternion &value);
    static int toIntFromString(const std::string &value);
    static float32 toFloat32FromString(const std::string &value);
    static Vector3 toVector3FromString(const std::string &value);
    static Vector4 toVector4FromString(const std::string &value);
    static Quaternion toQuaternionFromString(const std::string &value);

    XMLProject(IDelegate *delegate, Factory *factory, bool ownMemory);
    ~XMLProject();

    bool load(const char *path);
    bool load(const uint8 *data, vsize size);
    bool save(const char *path);
    void clear();

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

    void addModel(IModel *model, IRenderEngine *engine, const UUID &uuid, int order);
    void addMotion(IMotion *motion, const UUID &uuid);
    void removeModel(IModel *model);
    void removeMotion(IMotion *motion);

    void setGlobalSetting(const std::string &key, const std::string &value);
    void setModelSetting(const IModel *model, const std::string &key, const std::string &value);

private:
    struct PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(XMLProject)
};

} /* namespace extensions */
} /* namespace vpvl */

#endif
