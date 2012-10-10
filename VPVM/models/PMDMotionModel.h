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

#ifndef VPVM_PMDMOTIONMODEL_H
#define VPVM_PMDMOTIONMODEL_H

#include "models/MotionBaseModel.h"

#include <QtCore/QString>
#include <QtGui/QAbstractItemView>
#include <vpvl2/Common.h>

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
        State(const Scene *sceneRef, IModel *modelRef);
        ~State();
        void restore() const;
        void save();
        bool compact();
        void discard();
        void copyFrom(const State &value);
        void resetBones();
        void resetMorphs();
        IModel *model() const { return m_modelRef; }
        void setModel(IModel *value) { m_modelRef = value; }
    private:
        typedef QPair<Vector3, Quaternion> Transform;
        typedef QPair<IBone *, Transform> Bone;
        typedef QPair<IMorph *, Scalar> Morph;
        const Scene *m_sceneRef;
        IModel *m_modelRef;
        QList<Bone> m_bones;
        QList<Morph> m_morphs;
        Q_DISABLE_COPY(State)
    };

    explicit PMDMotionModel(QUndoGroup *undoRef, QObject *parent = 0);
    ~PMDMotionModel();

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    const QModelIndex frameIndexToModelIndex(ITreeItem *item, int timeIndex) const;

    virtual void saveTransform() = 0;
    virtual void commitTransform() = 0;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void updateModel(IModel *model, bool seek);
    void refreshModel(IModel *model);
    void setActiveUndoStack();
    int maxFrameIndex() const;
    bool forceCameraUpdate() const;
    void setSceneRef(const Scene *value);

    IModel *selectedModel() const { return m_modelRef; }
    const Keys keys() const { return m_keys[m_modelRef]; }
    const Scene *scene() const { return m_sceneRef; }

public slots:
    virtual void loadMotion(IMotion *motion, const IModel *model) = 0;
    virtual void setPMDModel(IModel *model) = 0;
    virtual void removeModel() = 0;
    void markAsNew(IModel *model);

signals:
    void modelDidChange(IModel *model);
    void motionDidUpdate(IModel *model);

protected:
    void removePMDModel(IModel *model);
    void removePMDMotion(IModel *model);
    void addPMDModel(IModel *model, const RootPtr &rootRef, const Keys &keys);
    bool hasPMDModel(IModel *model) const { return m_roots.contains(model); }
    const Values values() const { return m_values[m_modelRef]; }
    RootPtr rootPtr() const { return rootPtr(m_modelRef); }
    RootPtr rootPtr(IModel *model) const { return m_roots[model]; }
    ITreeItem *rootRef() const { return rootPtr().data(); }

    const Scene *m_sceneRef;
    IModel *m_modelRef;
    Vector3 m_lightDirection;

private:
    QHash<IModel *, Keys> m_keys;
    QHash<IModel *, Values> m_values;
    QHash<IModel *, RootPtr> m_roots;
    QHash<IModel *, UndoStackPtr> m_stacks;

    Q_DISABLE_COPY(PMDMotionModel)
};

}

#endif
