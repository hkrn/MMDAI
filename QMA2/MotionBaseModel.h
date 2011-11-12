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

#ifndef MOTIONBASEMODEL_H
#define MOTIONBASEMODEL_H

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

class MotionBaseModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    class ITreeItem
    {
    public:
        virtual void addChild(ITreeItem *item) = 0;
        virtual ITreeItem *parent() const = 0;
        virtual ITreeItem *child(int row) const = 0;
        virtual const QString &name() const = 0;
        virtual bool isRoot() const = 0;
        virtual bool isCategory() const = 0;
        virtual int rowIndex() const = 0;
        virtual int countChildren() const = 0;
    };

    enum DataRole
    {
        kNameRole = 0x1000,
        kBinaryDataRole
    };

    typedef QMap<QString, ITreeItem *> Keys;
    typedef QHash<QModelIndex, QVariant> Values;
    typedef QList<ITreeItem *> TreeItemList;
    typedef QSharedPointer<ITreeItem> RootPtr;
    typedef QSharedPointer<QUndoStack> UndoStackPtr;

    static const QVariant kInvalidData;
    static int toFrameIndex(const QModelIndex &index);
    static int toModelIndex(int frameIndex);

    MotionBaseModel(QUndoGroup *undo, QObject *parent = 0);
    virtual ~MotionBaseModel();

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    const QModelIndex frameIndexToModelIndex(ITreeItem *item, int frameIndex) const;

    virtual void saveMotion(vpvl::VMDMotion *motion) = 0;
    virtual void copyFrames(int frameIndex) = 0;
    virtual void startTransform() = 0;
    virtual void commitTransform() = 0;
    virtual void selectByModelIndex(const QModelIndex &index) = 0;
    virtual const QByteArray nameFromModelIndex(const QModelIndex &index) const = 0;
    void saveState();
    void restoreState();
    void discardState();
    void updateModel();
    void refreshModel();
    int maxFrameCount() const;

    vpvl::PMDModel *selectedModel() const { return m_model; }
    vpvl::VMDMotion *currentMotion() const { return m_motion; }
    void setFrameIndex(int value) { m_frameIndex = value; }
    void setModified(bool value) { m_modified = value; motionDidModify(value); }
    bool isModified() const { return m_modified; }
    const Keys keys() const { return m_keys[m_model]; }
    int currentFrameIndex() const { return m_frameIndex; }

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

public slots:
    void markAsNew(vpvl::PMDModel *model);
    virtual void setPMDModel(vpvl::PMDModel *model) = 0;
    virtual void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model) = 0;
    virtual void removeMotion() = 0;
    virtual void removeModel() = 0;
    virtual void deleteFrameByModelIndex(const QModelIndex &index) = 0;

signals:
    void modelDidChange(vpvl::PMDModel *model);
    void motionDidModify(bool value);
    void motionDidUpdate(vpvl::PMDModel *model);

protected:
    void addUndoCommand(QUndoCommand *command);
    void addPMDModel(vpvl::PMDModel *model, const RootPtr &root, const Keys &keys);
    void removePMDModel(vpvl::PMDModel *model);
    bool hasPMDModel(vpvl::PMDModel *model) const { return m_roots.contains(model); }
    const Values values() const { return m_values[m_model]; }
    RootPtr root() const { return root(m_model); }
    RootPtr root(vpvl::PMDModel *model) const { return m_roots[model]; }

    vpvl::PMDModel *m_model;
    vpvl::VMDMotion *m_motion;

private:
    vpvl::PMDModel::State *m_state;
    QHash<vpvl::PMDModel *, Keys> m_keys;
    QHash<vpvl::PMDModel *, Values> m_values;
    QHash<vpvl::PMDModel *, RootPtr> m_roots;
    QHash<vpvl::PMDModel *, UndoStackPtr> m_stacks;
    QUndoGroup *m_undo;
    int m_frameIndex;
    bool m_modified;

    Q_DISABLE_COPY(MotionBaseModel)
};

#endif // MOTIONBASEMODEL_H
