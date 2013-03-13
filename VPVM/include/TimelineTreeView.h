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

#ifndef VPVM_TIMELINETREEVIEW_H
#define VPVM_TIMELINETREEVIEW_H

#include <vpvl2/Common.h>

#include <QModelIndex>
#include <QHeaderView>
#include <QTreeView>

class QItemDelegate;

namespace vpvm
{

using namespace vpvl2;
class MotionBaseModel;

class TimelineTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit TimelineTreeView(MotionBaseModel *mbm, QItemDelegate *delegate, QWidget *parent = 0);
    ~TimelineTreeView();

    void initializeFrozenView();
    void selectFrameIndices(const QList<int> &frameIndices, bool registeredOnly);
    void deleteKeyframesBySelectedIndices();
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
    QItemSelectionModel *frozenViewSelectionModel() const;

public slots:
    void addKeyframesBySelectedIndices();
    void restoreExpandState();
    void updateFrozenTreeView();

signals:
    void modelIndexDidSelect(const QModelIndexList &indices);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

private slots:
    void addCollapsed(const QModelIndex &index);
    void addExpanded(const QModelIndex &index);
    void selectModelIndices(const QItemSelection &selected, const QItemSelection &deselected);
    void setBoneKeyframesWeightBySelectedIndices(const Vector3 &position, const Vector3 &rotation);
    void setMorphKeyframesWeightBySelectedIndices(float value);
    void updateSectionWidth(int logicalIndex, int newSize, int oldSize);

private:
    void updateFrozenTreeViewGeometry();

    QScopedPointer<QRubberBand> m_rubberBand;
    QScopedPointer<QTreeView> m_frozenTreeView;
    QRect m_rubberBandRect;
    QModelIndexList m_expanded;

    Q_DISABLE_COPY(TimelineTreeView)
};

class TimelineHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    explicit TimelineHeaderView(Qt::Orientation orientation,
                                bool stretchLastSection,
                                QWidget *parent = 0);
    virtual ~TimelineHeaderView();

signals:
    void timeIndexDidSelect(int timeIndex);

protected:
    void mousePressEvent(QMouseEvent *e);

private:
    Q_DISABLE_COPY(TimelineHeaderView)
};

} /* namespace vpvm */

#endif // TIMELINETREEVIEW_H
