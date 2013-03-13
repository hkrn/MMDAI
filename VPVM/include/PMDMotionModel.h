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

#ifndef VPVM_PMDMOTIONMODEL_H
#define VPVM_PMDMOTIONMODEL_H

#include "MotionBaseModel.h"

#include <vpvl2/Common.h>

#include <QString>
#include <QAbstractItemView>

namespace vpvl2 {
class IBone;
class IModel;
class IMorph;
class IMotion;
class Scene;
}

class QUndoCommand;
class QUndoGroup;
class QUndoStack;

namespace vpvm
{

using namespace vpvl2;

class PMDMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    class State {
    public:
        State(const Scene *scene, IModelSharedPtr model);
        ~State();
        void restore() const;
        void save();
        bool compact();
        void discard();
        void copyFrom(const State &value);
        void resetBones();
        void resetMorphs();
        IModelSharedPtr model() const { return m_modelRef; }
        void setModelRef(IModelSharedPtr value) { m_modelRef = value; }
        void setSceneRef(const Scene *value) { m_sceneRef = value; }
    private:
        typedef QPair<Vector3, Quaternion> Transform;
        typedef QPair<IBone *, Transform> Bone;
        typedef QPair<IMorph *, Scalar> Morph;
        const Scene *m_sceneRef;
        IModelSharedPtr m_modelRef;
        QList<Bone> m_bones;
        QList<Morph> m_morphs;
        Q_DISABLE_COPY(State)
    };

    explicit PMDMotionModel(QUndoGroup *undoRef, QObject *parent = 0);
    ~PMDMotionModel();

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    const QModelIndex timeIndexToModelIndex(ITreeItem *item, int timeIndex) const;

    virtual void saveTransform() = 0;
    virtual void commitTransform() = 0;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void updateModel(IModelSharedPtr model, bool seek);
    void refreshModel(IModelSharedPtr model);
    void activateUndoStack();
    int maxTimeIndex() const;
    bool forceCameraUpdate() const;
    virtual IMotionSharedPtr currentMotionRef() const;
    virtual void setSceneRef(const Scene *value);

    IModelSharedPtr selectedModel() const { return m_modelRef; }
    const Keys keys() const { return m_keys[m_modelRef]; }
    const Scene *scene() const { return m_sceneRef; }

public slots:
    virtual void loadMotion(IMotionSharedPtr motion, const IModelSharedPtr model) = 0;
    virtual void setPMDModel(IModelSharedPtr model) = 0;
    virtual void removeModel() = 0;
    void markAsNew(IModelSharedPtr model);

signals:
    void modelDidChange(IModelSharedPtr model);
    void motionDidUpdate(IModelSharedPtr model);

protected:
    void removePMDModel(IModelSharedPtr model);
    void removePMDMotion(IModelSharedPtr model);
    void addPMDModel(IModelSharedPtr model, const RootPtr &rootRef, const Keys &keys);
    void addPMDModelMotion(const IModelSharedPtr model, IMotionSharedPtr motion);
    bool hasPMDModel(IModelSharedPtr model) const { return m_roots.contains(model); }
    const Values values() const { return m_values[m_modelRef]; }
    RootPtr rootPtr() const { return rootPtr(m_modelRef); }
    RootPtr rootPtr(IModelSharedPtr model) const { return m_roots[model]; }
    ITreeItem *rootRef() const { return rootPtr().data(); }

    const Scene *m_sceneRef;
    IModelSharedPtr m_modelRef;
    Vector3 m_lightDirection;

private:
    void activateUndoStack(const IModelSharedPtr model);

    QHash<const IModelSharedPtr, IMotionSharedPtr> m_motionRefs;
    QHash<const IModelSharedPtr, Keys> m_keys;
    QHash<const IModelSharedPtr, Values> m_values;
    QHash<const IModelSharedPtr, RootPtr> m_roots;
    QHash<const IModelSharedPtr, UndoStackPtr> m_stacks;

    Q_DISABLE_COPY(PMDMotionModel)
};

}

#endif
