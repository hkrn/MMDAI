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

#ifndef VPVM_TIMELINEWIDGET_H
#define VPVM_TIMELINEWIDGET_H

#include <QtCore/QModelIndex>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif
#include "vpvl2/IKeyframe.h"

class QLabel;
class QPushButton;
class QSettings;
class QSpinBox;
class QTreeView;

namespace vpvm
{

using namespace vpvl2;
class MotionBaseModel;
class TimelineHeaderView;
class TimelineTreeView;

class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    static const int kTimeIndexColumnMax = 2592000; /* 30frame * 3600sec * 24h */

    explicit TimelineWidget(MotionBaseModel *base,
                            bool stretchLastSection,
                            QWidget *parent = 0);
    ~TimelineWidget();

    int currentTimeIndex() const;
    int selectedTimeIndex() const;
    void setTimeIndexSpinBoxEnable(bool value);

    TimelineTreeView *treeViewRef() const { return m_treeView.data(); }

public slots:
    void setCurrentTimeIndex(const IKeyframe::TimeIndex &timeIndex);
    void setCurrentTimeIndex(int timeIndex, bool forceSeek = false);

signals:
    void motionDidSeek(const IKeyframe::TimeIndex &column, bool forceCameraUpdate, bool forceEvenSame);

private slots:
    void retranslate();
    void setCurrentTimeIndexBySpinBox();
    void setCurrentTimeIndexAndExpandBySpinBox();
    void setCurrentTimeIndexAndSelect(int timeIndex);
    void setCurrentTimeIndex(const QModelIndex &index, bool forceSeek = false);
    void adjustFrameColumnSize(int value);

private:
    QScopedPointer<TimelineTreeView> m_treeView;
    QScopedPointer<TimelineHeaderView> m_headerView;
    QScopedPointer<QLabel> m_label;
    QScopedPointer<QPushButton> m_button;
    QScopedPointer<QSpinBox> m_spinBox;
    QSettings *m_settingsRef;
    QModelIndex m_index;

    Q_DISABLE_COPY(TimelineWidget)
};

} /* namespace vpvm */

#endif // TIMLINEWIDGET_H
