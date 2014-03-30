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

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
class IModel;
class IMotion;
class IRenderEngine;

namespace extensions
{
namespace vpdb
{

class VPVL2_API Project VPVL2_DECL_FINAL : public Scene {
public:
    typedef std::string UUID;
    typedef std::vector<UUID> UUIDList;
    typedef std::map<UUID, StringMap> ModelSettings;

    Project(bool ownMemory);
    ~Project();

    bool load(const char *filename);
    bool save(const char *filename);
    void clear();

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

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace vpdb */
} /* namespace extensions */
} /* namespace VPVL2_VERSION_NS */
using namespace VPVL2_VERSION_NS;

} /* namespace vpvl2 */
