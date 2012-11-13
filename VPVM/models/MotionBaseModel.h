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

#ifndef VPVM_MOTIONBASEMODEL_H
#define VPVM_MOTIONBASEMODEL_H

#include <QtCore/QString>
#include <QtCore/QVariant>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QUndoStack>
#include <QtWidgets/QUndoGroup>
#else
#include <QtGui/QAbstractItemView>
#include <QtGui/QUndoStack>
#include <QtGui/QUndoGroup>
#endif
#include "vpvl2/IKeyframe.h"

namespace vpvl2 {
class IMotion;
}

namespace vpvm
{

using namespace vpvl2;

class MotionBaseModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    class ITreeItem
    {
    public:
        virtual ~ITreeItem() {}
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
    static const int kFrameIndexColumnStep = 5;
    static const int kFrameIndexColumnMinimum = 31; /* 1 for header */

    static int toTimeIndex(const QModelIndex &index);
    static int toTimeIndex(int modelColumnIndex);
    static int toModelIndex(int timeIndex);

    MotionBaseModel(QUndoGroup *undo, QObject *parent = 0);
    virtual ~MotionBaseModel();

    QVariant headerData(int /* section */, Qt::Orientation /* orientation */, int /* role */) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    void cutKeyframesByModelIndices(const QModelIndexList &indices, int timeIndex);
    QItemSelection selectKeyframesFromItemSelection(const QItemSelection &selection);

    virtual const QModelIndex frameIndexToModelIndex(ITreeItem *item, int timeIndex) const = 0;
    virtual const QString nameFromModelIndex(const QModelIndex &index) const = 0;
    virtual void saveMotion(IMotion *motion) = 0;
    virtual void copyKeyframesByModelIndices(const QModelIndexList &indices, int timeIndex) = 0;
    virtual void pasteKeyframesByTimeIndex(int timeIndex) = 0;
    virtual int maxFrameIndex() const = 0;
    virtual bool forceCameraUpdate() const = 0;

    IMotion *currentMotionRef() const { return m_motionRef; }
    void setTimeIndex(const IKeyframe::TimeIndex &newIndex);
    void setModified(bool value);
    bool isModified() const { return m_modified; }
    int maxFrameCount() const { return m_frameIndexColumnOffset; }
    const IKeyframe::TimeIndex &timeIndex() const { return m_timeIndex; }
    bool canFetchMore(const QModelIndex & /* parent */) const;
    void fetchMore(const QModelIndex &parent);
    int frameIndexColumnMax() const;
    void setFrameIndexColumnMax(int newValue);
    void setFrameIndexColumnMax(const IMotion *motion);
    void updateFrameIndexColumnMax();

public slots:
    virtual void removeMotion() = 0;
    virtual void addKeyframesByModelIndices(const QModelIndexList &indices) = 0;
    virtual void deleteKeyframesByModelIndices(const QModelIndexList &indices) = 0;

signals:
    void motionDidModify(bool value);
    void timeIndexDidChange(const IKeyframe::TimeIndex &newFrameIndex, const IKeyframe::TimeIndex &oldFrameIndex);
    void frameIndexColumnMaxDidChange(int newValue, int oldValue);

protected:
    virtual ITreeItem *rootRef() const = 0;
    void addUndoCommand(QUndoCommand *command);
    void resetModel();

    IMotion *m_motionRef;
    QUndoGroup *m_undoRef;
    IKeyframe::TimeIndex m_timeIndex;
    int m_frameIndexColumnMax;
    int m_frameIndexColumnOffset;
    bool m_modified;

private:
    void setFrameIndexColumnMax0(int newValue);

    Q_DISABLE_COPY(MotionBaseModel)
};

} /* namespace vpvl2 */

#endif // MOTIONBASEMODEL_H
