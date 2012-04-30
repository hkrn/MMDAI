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

#ifndef MORPHMOTIONMODEL_H
#define MORPHMOTIONMODEL_H

#include "models/PMDMotionModel.h"

#include <vpvl/BaseAnimation.h>

namespace vpvl2 {
class Factory;
class IModel;
class IMorph;
class IMorphKeyframe;
}

class MorphMotionModel : public PMDMotionModel
{
    Q_OBJECT

public:
    typedef QSharedPointer<vpvl2::IMorphKeyframe> KeyFramePtr;
    typedef QPair<int, KeyFramePtr> KeyFramePair;
    typedef QList<KeyFramePair> KeyFramePairList;

    explicit MorphMotionModel(vpvl2::Factory *factory, QUndoGroup *undo = 0, QObject *parent = 0);
    ~MorphMotionModel();

    void saveMotion(vpvl2::IMotion *motion);
    void copyKeyframesByModelIndices(const QModelIndexList &indices, int frameIndex);
    void pasteKeyframesByFrameIndex(int frameIndex);
    void applyKeyframeWeightByModelIndices(const QModelIndexList &indices, float value);
    const QString nameFromModelIndex(const QModelIndex &index) const;

    void setFrames(const KeyFramePairList &frames);
    void resetAllMorphs();
    vpvl2::IMorph *findMorph(const QString &name);
    void setWeight(float value);
    void setWeight(const vpvl2::Scalar &value, vpvl2::IMorph *morph);
    vpvl2::IMorph *selectedMorph() const { return m_selectedMorphs.isEmpty() ? 0 : m_selectedMorphs.first(); }
    bool isMorphSelected() const { return m_model != 0 && selectedMorph() != 0; }
    vpvl2::Factory *factory() const { return m_factory; }

public slots:
    void addKeyframesByModelIndices(const QModelIndexList &indices);
    void selectKeyframesByModelIndices(const QModelIndexList &indices);
    void deleteKeyframesByModelIndices(const QModelIndexList &indices);
    void removeModel();
    void removeMotion();
    void setPMDModel(vpvl2::IModel *model);
    void loadMotion(vpvl2::IMotion *motion, vpvl2::IModel *model);
    void selectMorphs(const QList<vpvl2::IMorph *> &morphs);
    void saveTransform();
    void commitTransform();

signals:
    void morphsDidSelect(const QList<vpvl2::IMorph *> &morphs);

private:
    vpvl2::Factory *m_factory;
    QList<vpvl2::IMorph *> m_selectedMorphs;
    KeyFramePairList m_copiedKeyframes;
    PMDMotionModel::State m_state;

    Q_DISABLE_COPY(MorphMotionModel)
};

#endif // MORPHMOTIONMODEL_H
