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

#ifndef PMDMOTIONMODEL_H
#define PMDMOTIONMODEL_H

#include "models/MotionBaseModel.h"

#include <QtCore/QString>
#include <QtGui/QAbstractItemView>
#include <vpvl2/Common.h>

namespace vpvl2 {
class IBone;
class IModel;
class IMorph;
class IMotion;
}

class QUndoCommand;
class QUndoGroup;
class QUndoStack;

class PMDMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    class State {
    public:
        State(vpvl2::IModel *model, PMDMotionModel *parent);
        ~State();
        void restore() const;
        void save();
        bool compact();
        void discard();
        void copyFrom(const State &value);
        void resetBones();
        void resetMorphs();
        vpvl2::IModel *model() const { return m_model; }
        void setModel(vpvl2::IModel *value) { m_model = value; }
    private:
        typedef QPair<vpvl2::Vector3, vpvl2::Quaternion> Transform;
        typedef QPair<vpvl2::IBone *, Transform> Bone;
        typedef QPair<vpvl2::IMorph *, vpvl2::Scalar> Morph;
        PMDMotionModel *m_parent;
        vpvl2::IModel *m_model;
        QList<Bone> m_bones;
        QList<Morph> m_morphs;
        Q_DISABLE_COPY(State)
    };

    explicit PMDMotionModel(QUndoGroup *undo, QObject *parent = 0);
    ~PMDMotionModel();

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    const QModelIndex frameIndexToModelIndex(ITreeItem *item, int timeIndex) const;

    virtual void saveTransform() = 0;
    virtual void commitTransform() = 0;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void updateModel(vpvl2::IModel *model, bool seek);
    void refreshModel(vpvl2::IModel *model);
    void setActiveUndoStack();
    int maxFrameIndex() const;
    bool forceCameraUpdate() const;

    vpvl2::IModel *selectedModel() const { return m_model; }
    const Keys keys() const { return m_keys[m_model]; }
    const vpvl2::Vector3 &lightDirection() const { return m_lightDirection; }

public slots:
    virtual void loadMotion(vpvl2::IMotion *motion, vpvl2::IModel *model) = 0;
    virtual void setPMDModel(vpvl2::IModel *model) = 0;
    virtual void removeModel() = 0;
    void markAsNew(vpvl2::IModel *model);
    void setLightDirection(const vpvl2::Vector3 &value);

signals:
    void modelDidChange(vpvl2::IModel *model);
    void motionDidUpdate(vpvl2::IModel *model);

protected:
    void removePMDModel(vpvl2::IModel *model);
    void removePMDMotion(vpvl2::IModel *model);
    void addPMDModel(vpvl2::IModel *model, const RootPtr &root, const Keys &keys);
    bool hasPMDModel(vpvl2::IModel *model) const { return m_roots.contains(model); }
    const Values values() const { return m_values[m_model]; }
    RootPtr rootPtr() const { return rootPtr(m_model); }
    RootPtr rootPtr(vpvl2::IModel *model) const { return m_roots[model]; }
    ITreeItem *root() const { return rootPtr().data(); }

    vpvl2::IModel *m_model;
    vpvl2::Vector3 m_lightDirection;

private:
    QHash<vpvl2::IModel *, Keys> m_keys;
    QHash<vpvl2::IModel *, Values> m_values;
    QHash<vpvl2::IModel *, RootPtr> m_roots;
    QHash<vpvl2::IModel *, UndoStackPtr> m_stacks;

    Q_DISABLE_COPY(PMDMotionModel)
};

#endif
