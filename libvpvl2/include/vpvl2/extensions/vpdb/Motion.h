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

#include <vpvl2/IMotion.h>

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace extensions
{
namespace vpdb
{

class Project;

class VPVL2_API Motion VPVL2_DECL_FINAL : public IMotion {
public:
    Motion(Project *parent);
    ~Motion();

    bool load(const uint8 *data, vsize size);
    void save(uint8 *data) const;
    vsize estimateSize() const;
    Error error() const;
    IModel *parentModelRef() const;
    void refresh();
    void seekSeconds(const float64 &seconds);
    void seekSceneSeconds(const float64 &seconds, Scene *scene, int flags);
    void seekTimeIndex(const IKeyframe::TimeIndex &timeIndex);
    void seekSceneTimeIndex(const IKeyframe::TimeIndex &timeIndex, Scene *scene, int flags);
    void reset();
    float64 durationSeconds() const;
    IKeyframe::TimeIndex durationTimeIndex() const;
    bool isReachedTo(const IKeyframe::TimeIndex &timeIndex) const;
    IBoneKeyframe *createBoneKeyframe();
    ICameraKeyframe *createCameraKeyframe();
    IEffectKeyframe *createEffectKeyframe();
    ILightKeyframe *createLightKeyframe();
    IModelKeyframe *createModelKeyframe();
    IMorphKeyframe *createMorphKeyframe();
    IProjectKeyframe *createProjectKeyframe();
    void addKeyframe(IKeyframe *value);
    int countKeyframes(IKeyframe::Type value) const;
    IKeyframe::LayerIndex countLayers(const IString *name,
                                      IKeyframe::Type type) const;
    void getKeyframeRefs(const IKeyframe::TimeIndex &timeIndex,
                         const IKeyframe::LayerIndex &layerIndex,
                         IKeyframe::Type type,
                         Array<IKeyframe *> &keyframes);
    IBoneKeyframe *findBoneKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                       const IString *name,
                                       const IKeyframe::LayerIndex &layerIndex) const;
    IBoneKeyframe *findBoneKeyframeRefAt(int index) const;
    ICameraKeyframe *findCameraKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                           const IKeyframe::LayerIndex &layerIndex) const;
    ICameraKeyframe *findCameraKeyframeRefAt(int index) const;

    IEffectKeyframe *findEffectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                           const IString *name,
                                           const IKeyframe::LayerIndex &layerIndex) const;
    IEffectKeyframe *findEffectKeyframeRefAt(int index) const;
    virtual ILightKeyframe *findLightKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                                 const IKeyframe::LayerIndex &layerIndex) const;
    ILightKeyframe *findLightKeyframeRefAt(int index) const;
    IModelKeyframe *findModelKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                         const IKeyframe::LayerIndex &layerIndex) const;
    IModelKeyframe *findModelKeyframeRefAt(int index) const;
    IMorphKeyframe *findMorphKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                         const IString *name,
                                         const IKeyframe::LayerIndex &layerIndex) const;
    IMorphKeyframe *findMorphKeyframeRefAt(int index) const;
    IProjectKeyframe *findProjectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                             const IKeyframe::LayerIndex &layerIndex) const;
    IProjectKeyframe *findProjectKeyframeRefAt(int index) const;
    void replaceKeyframe(IKeyframe *value, bool alsoDelete);
    void removeKeyframe(IKeyframe *value);
    void deleteKeyframe(IKeyframe *&value);
    void update(IKeyframe::Type type);
    void getAllKeyframeRefs(Array<IKeyframe *> &value, IKeyframe::Type type);
    void setAllKeyframes(const Array<IKeyframe *> &value, IKeyframe::Type type);
    void createFirstKeyframesUnlessFound();
    IMotion *clone() const;
    Scene *parentSceneRef() const;
    const IString *name() const;
    FormatType type() const;

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace vpdb */
} /* namespace extensions */
} /* namespace VPVL2_VERSION_NS */
using namespace VPVL2_VERSION_NS;

} /* namespace vpvl2 */
