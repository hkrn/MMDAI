/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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
#include <vpvl/PMDModel.h>

namespace vpvl {
class VMDMotion;
class VPDPose;
}

class QUndoCommand;
class QUndoGroup;
class QUndoStack;

class PMDMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    explicit PMDMotionModel(QUndoGroup *undo, QObject *parent = 0);
    virtual ~PMDMotionModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual const QModelIndex frameIndexToModelIndex(ITreeItem *item, int frameIndex) const;

    virtual void copyFrames(int frameIndex) = 0;
    virtual void startTransform() = 0;
    virtual void commitTransform() = 0;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void saveState();
    void restoreState();
    void discardState();
    void updateModel();
    void refreshModel();
    int maxFrameCount() const;

    vpvl::PMDModel *selectedModel() const { return m_model; }
    const Keys keys() const { return m_keys[m_model]; }

public slots:
    virtual void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model) = 0;
    virtual void setPMDModel(vpvl::PMDModel *model) = 0;
    virtual void removeModel() = 0;
    void markAsNew(vpvl::PMDModel *model);

signals:
    void modelDidChange(vpvl::PMDModel *model);
    void motionDidUpdate(vpvl::PMDModel *model);

protected:
    void removePMDModel(vpvl::PMDModel *model);
    void removePMDMotion(vpvl::PMDModel *model);
    void addPMDModel(vpvl::PMDModel *model, const RootPtr &root, const Keys &keys);
    bool hasPMDModel(vpvl::PMDModel *model) const { return m_roots.contains(model); }
    const Values values() const { return m_values[m_model]; }
    RootPtr rootPtr() const { return rootPtr(m_model); }
    RootPtr rootPtr(vpvl::PMDModel *model) const { return m_roots[model]; }
    virtual ITreeItem *root() const { return rootPtr().data(); }

    vpvl::PMDModel *m_model;

private:
    vpvl::PMDModel::State *m_state;
    QHash<vpvl::PMDModel *, Keys> m_keys;
    QHash<vpvl::PMDModel *, Values> m_values;
    QHash<vpvl::PMDModel *, RootPtr> m_roots;
    QHash<vpvl::PMDModel *, UndoStackPtr> m_stacks;

    Q_DISABLE_COPY(PMDMotionModel)
};

#endif
