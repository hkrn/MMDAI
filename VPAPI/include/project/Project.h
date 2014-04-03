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

#include <vpvl2/Common.h>
#include <vpvl2/extensions/StringMap.h>

#include <QUuid>
#include <QList>
#include <QHash>
#include <QSqlDatabase>

namespace vpvl2 {
namespace VPVL2_VERSION_NS {
class IModel;
class IMotion;
class IRenderEngine;
}
}

namespace project
{

class Project : public vpvl2::Scene {
public:
    typedef QUuid UUID;
    typedef QList<UUID> UUIDList;
    typedef QHash<UUID, vpvl2::extensions::StringMap> ModelSettings;

    Project(QSqlDatabase *database, bool ownMemory);
    ~Project();

    bool load(const char *filename);
    bool save(const char *filename);
    void clear();

    const UUIDList modelUUIDs() const;
    const UUIDList motionUUIDs() const;
    UUID modelUUID(const vpvl2::IModel *model) const;
    UUID motionUUID(const vpvl2::IMotion *motion) const;
    vpvl2::IModel *findModel(const UUID &uuid) const;
    vpvl2::IMotion *findMotion(const UUID &uuid) const;
    bool containsModel(const vpvl2::IModel *model) const;
    bool containsMotion(const vpvl2::IMotion *motion) const;
    bool isDirty() const;
    void setDirty(bool value);

    void addModel(vpvl2::IModel *model, vpvl2::IRenderEngine *engine, const UUID &uuid, int order);
    void addMotion(vpvl2::IMotion *motion, const UUID &uuid);
    void removeModel(vpvl2::IModel *model);
    void removeMotion(vpvl2::IMotion *motion);

    QSqlDatabase *databaseHandle() const;

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace project */
