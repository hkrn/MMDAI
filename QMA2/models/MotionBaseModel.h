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

#ifndef MOTIONBASEMODEL_H
#define MOTIONBASEMODEL_H

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtGui/QAbstractItemView>
#include <QtGui/QUndoStack>
#include <QtGui/QUndoGroup>

namespace vpvl {
class VMDMotion;
}

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

    typedef QMap<QString, ITreeItem *> Keys;
    typedef QHash<QModelIndex, QVariant> Values;
    typedef QList<ITreeItem *> TreeItemList;
    typedef QSharedPointer<ITreeItem> RootPtr;
    typedef QSharedPointer<QUndoStack> UndoStackPtr;

    enum DataRole
    {
        kNameRole = 0x1000,
        kBinaryDataRole
    };

    static int toFrameIndex(const QModelIndex &index) {
        return toFrameIndex(index.column());
    }
    static int toFrameIndex(int modelColumnIndex) {
        // column index 0 is row header
        return qMax(modelColumnIndex - 1, 0);
    }
    static int toModelIndex(int frameIndex) {
        // column index 0 is row header
        return qMax(frameIndex + 1, 0);
    }

    MotionBaseModel(QUndoGroup *undo, QObject *parent = 0)
        : QAbstractTableModel(parent),
          m_motion(0),
          m_undo(undo),
          m_frameIndex(0.0),
          m_modified(false)
    {
    }
    virtual ~MotionBaseModel() {
    }

    virtual QVariant headerData(int /* section */, Qt::Orientation /* orientation */, int /* role */) const {
        return QVariant();
    }
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const {
        if (!hasIndex(row, column, parent))
            return QModelIndex();
        ITreeItem *parentItem = 0;
        if (!parent.isValid())
            parentItem = root();
        else
            parentItem = static_cast<ITreeItem *>(parent.internalPointer());
        ITreeItem *childItem = parentItem->child(row);
        return childItem ? createIndex(row, column, childItem) : QModelIndex();
    }
    virtual QModelIndex parent(const QModelIndex &child) const {
        if (!child.isValid())
            return QModelIndex();
        ITreeItem *childItem = static_cast<ITreeItem *>(child.internalPointer());
        ITreeItem *parentItem = childItem->parent();
        return parentItem == root() ? QModelIndex() : createIndex(parentItem->rowIndex(), 0, parentItem);
    }
    virtual int rowCount(const QModelIndex &parent) const {
        ITreeItem *parentItem;
        if (parent.column() > 0)
            return 0;
        if (!parent.isValid()) {
            parentItem = root();
            if (!parentItem)
                return 0;
        }
        else {
            parentItem = static_cast<ITreeItem *>(parent.internalPointer());
        }
        return parentItem->countChildren();
    }
    virtual void selectKeyframesByModelIndex(const QModelIndex &index) {
        QModelIndexList indices;
        indices.append(index);
        selectKeyframesByModelIndices(indices);
    }

    virtual const QModelIndex frameIndexToModelIndex(ITreeItem *item, int frameIndex) const = 0;
    virtual const QByteArray nameFromModelIndex(const QModelIndex &index) const = 0;
    virtual void saveMotion(vpvl::VMDMotion *motion) = 0;
    virtual void copyKeyframes(int frameIndex) = 0;
    virtual void pasteKeyframes(int frameIndex) = 0;
    virtual int maxFrameCount() const = 0;
    virtual int maxFrameIndex() const = 0;

    vpvl::VMDMotion *currentMotion() const { return m_motion; }
    void setFrameIndex(float newIndex) {
        float oldIndex = m_frameIndex;
        m_frameIndex = newIndex;
        emit frameIndexDidChange(newIndex, oldIndex);
    }
    void setModified(bool value) {
        m_modified = value;
        emit motionDidModify(value);
    }
    bool isModified() const { return m_modified; }
    float frameIndex() const { return m_frameIndex; }

public slots:
    virtual void removeMotion() = 0;
    virtual void addKeyframesByModelIndices(const QModelIndexList &indices) = 0;
    virtual void selectKeyframesByModelIndices(const QModelIndexList &indices) = 0;
    virtual void deleteKeyframesByModelIndices(const QModelIndexList &indices) = 0;
    virtual void applyKeyframeWeightByModelIndices(const QModelIndexList &indices, float value) = 0;

signals:
    void motionDidModify(bool value);
    void frameIndexDidChange(int newFrameIndex, int oldFrameIndex);

protected:
    virtual ITreeItem *root() const = 0;
    void addUndoCommand(QUndoCommand *command) {
        QUndoStack *activeStack = m_undo->activeStack();
        if (activeStack)
            activeStack->push(command);
    }

    vpvl::VMDMotion *m_motion;
    QUndoGroup *m_undo;
    float m_frameIndex;
    bool m_modified;

private:
    Q_DISABLE_COPY(MotionBaseModel)
};

#endif // MOTIONBASEMODEL_H
