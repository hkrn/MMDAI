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

#ifndef VPVM_MORPHMOTIONMODEL_H
#define VPVM_MORPHMOTIONMODEL_H

#include "models/PMDMotionModel.h"

#include <vpvl2/IModel.h>
#include <vpvl2/IMorph.h>

namespace vpvl2 {
class Factory;
class IMorphKeyframe;
}

namespace vpvm
{

using namespace vpvl2;

class MorphMotionModel : public PMDMotionModel
{
    Q_OBJECT

public:
    typedef QSharedPointer<IMorphKeyframe> KeyFramePtr;
    typedef QPair<int, KeyFramePtr> KeyFramePair;
    typedef QList<KeyFramePair> KeyFramePairList;

    explicit MorphMotionModel(Factory *factoryRef, QUndoGroup *undoRef = 0, QObject *parent = 0);
    ~MorphMotionModel();

    void saveMotion(IMotion *motion);
    void copyKeyframesByModelIndices(const QModelIndexList &indices, int timeIndex);
    void pasteKeyframesByTimeIndex(int timeIndex);
    void applyKeyframeWeightByModelIndices(const QModelIndexList &indices, float value);
    void selectMorphsByModelIndices(const QModelIndexList &indices);
    bool isSelectionIdentical(const QList<IMorph *> &morphs);
    const QString nameFromModelIndex(const QModelIndex &index) const;
    const QModelIndexList modelIndicesFromMorphs(const QList<IMorph *> &morphs, int timeIndex) const;

    void setKeyframes(const KeyFramePairList &keyframes);
    void setWeight(IMorph::WeightPrecision &value);
    void setWeight(const IMorph::WeightPrecision &value, IMorph *morph);
    void setSceneRef(const Scene *value);
    IMorph *selectedMorph() const { return m_selectedMorphs.isEmpty() ? 0 : m_selectedMorphs.first(); }
    bool isMorphSelected() const { return m_modelRef != 0 && selectedMorph() != 0; }
    Factory *factoryRef() const { return m_factoryRef; }

public slots:
    void addKeyframesByModelIndices(const QModelIndexList &indices);
    void selectKeyframesByModelIndices(const QModelIndexList &indices);
    void deleteKeyframesByModelIndices(const QModelIndexList &indices);
    void removeModel();
    void removeMotion();
    void setPMDModel(IModelSharedPtr model);
    void loadMotion(IMotionSharedPtr motion, const IModelSharedPtr model);
    void selectMorphs(const QList<IMorph *> &morphs);
    void saveTransform();
    void commitTransform();
    void resetAllMorphs();

signals:
    void morphsDidSelect(const QList<IMorph *> &morphs);

private:
    Factory *m_factoryRef;
    QList<IMorph *> m_selectedMorphs;
    KeyFramePairList m_copiedKeyframes;
    PMDMotionModel::State m_state;

    Q_DISABLE_COPY(MorphMotionModel)
};

} /* namespace vpvl2 */

#endif // MORPHMOTIONMODEL_H
