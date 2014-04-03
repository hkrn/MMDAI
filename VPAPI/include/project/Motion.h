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

namespace project
{
class Project;

class Motion : public vpvl2::IMotion {
public:
    Motion(Project *parent);
    ~Motion();

    bool load(const vpvl2::uint8 *data, vpvl2::vsize size);
    void save(vpvl2::uint8 *data) const;
    vpvl2::vsize estimateSize() const;
    vpvl2::IMotion::Error error() const;
    vpvl2::IModel *parentModelRef() const;
    void refresh();
    void seekSeconds(const vpvl2::float64 &seconds);
    void seekSceneSeconds(const vpvl2::float64 &seconds, vpvl2::Scene *scene, int flags);
    void seekTimeIndex(const vpvl2::IKeyframe::TimeIndex &timeIndex);
    void seekSceneTimeIndex(const vpvl2::IKeyframe::TimeIndex &timeIndex, vpvl2::Scene *scene, int flags);
    void reset();
    vpvl2::float64 durationSeconds() const;
    vpvl2::IKeyframe::TimeIndex durationTimeIndex() const;
    bool isReachedTo(const vpvl2::IKeyframe::TimeIndex &timeIndex) const;
    vpvl2::IBoneKeyframe *createBoneKeyframe();
    vpvl2::ICameraKeyframe *createCameraKeyframe();
    vpvl2::IEffectKeyframe *createEffectKeyframe();
    vpvl2::ILightKeyframe *createLightKeyframe();
    vpvl2::IModelKeyframe *createModelKeyframe();
    vpvl2::IMorphKeyframe *createMorphKeyframe();
    vpvl2::IProjectKeyframe *createProjectKeyframe();
    void addKeyframe(vpvl2::IKeyframe *value);
    int countKeyframes(vpvl2::IKeyframe::Type value) const;
    vpvl2::IKeyframe::LayerIndex countLayers(const vpvl2::IString *name,
                                             vpvl2::IKeyframe::Type type) const;
    void getKeyframeRefs(const vpvl2::IKeyframe::TimeIndex &timeIndex,
                         const vpvl2::IKeyframe::LayerIndex &layerIndex,
                         vpvl2::IKeyframe::Type type,
                         vpvl2::Array<vpvl2::IKeyframe *> &keyframes);
    vpvl2::IBoneKeyframe *findBoneKeyframeRef(const vpvl2::IKeyframe::TimeIndex &timeIndex,
                                              const vpvl2::IString *name,
                                              const vpvl2::IKeyframe::LayerIndex &layerIndex) const;
    vpvl2::IBoneKeyframe *findBoneKeyframeRefAt(int index) const;
    vpvl2::ICameraKeyframe *findCameraKeyframeRef(const vpvl2::IKeyframe::TimeIndex &timeIndex,
                                                  const vpvl2::IKeyframe::LayerIndex &layerIndex) const;
    vpvl2::ICameraKeyframe *findCameraKeyframeRefAt(int index) const;

    vpvl2::IEffectKeyframe *findEffectKeyframeRef(const vpvl2::IKeyframe::TimeIndex &timeIndex,
                                                  const vpvl2::IString *name,
                                                  const vpvl2::IKeyframe::LayerIndex &layerIndex) const;
    vpvl2::IEffectKeyframe *findEffectKeyframeRefAt(int index) const;
    vpvl2::ILightKeyframe *findLightKeyframeRef(const vpvl2::IKeyframe::TimeIndex &timeIndex,
                                                const vpvl2::IKeyframe::LayerIndex &layerIndex) const;
    vpvl2::ILightKeyframe *findLightKeyframeRefAt(int index) const;
    vpvl2::IModelKeyframe *findModelKeyframeRef(const vpvl2::IKeyframe::TimeIndex &timeIndex,
                                                const vpvl2::IKeyframe::LayerIndex &layerIndex) const;
    vpvl2::IModelKeyframe *findModelKeyframeRefAt(int index) const;
    vpvl2::IMorphKeyframe *findMorphKeyframeRef(const vpvl2::IKeyframe::TimeIndex &timeIndex,
                                                const vpvl2::IString *name,
                                                const vpvl2::IKeyframe::LayerIndex &layerIndex) const;
    vpvl2::IMorphKeyframe *findMorphKeyframeRefAt(int index) const;
    vpvl2::IProjectKeyframe *findProjectKeyframeRef(const vpvl2::IKeyframe::TimeIndex &timeIndex,
                                                    const vpvl2::IKeyframe::LayerIndex &layerIndex) const;
    vpvl2::IProjectKeyframe *findProjectKeyframeRefAt(int index) const;
    void replaceKeyframe(vpvl2::IKeyframe *value, bool alsoDelete);
    void removeKeyframe(vpvl2::IKeyframe *value);
    void deleteKeyframe(vpvl2::IKeyframe *&value);
    void update(vpvl2::IKeyframe::Type type);
    void getAllKeyframeRefs(vpvl2::Array<vpvl2::IKeyframe *> &value, vpvl2::IKeyframe::Type type);
    void setAllKeyframes(const vpvl2::Array<vpvl2::IKeyframe *> &value, vpvl2::IKeyframe::Type type);
    void createFirstKeyframesUnlessFound();
    vpvl2::IMotion *clone() const;
    vpvl2::Scene *parentSceneRef() const;
    const vpvl2::IString *name() const;
    vpvl2::IMotion::FormatType type() const;

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace project */
