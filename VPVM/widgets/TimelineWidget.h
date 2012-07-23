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

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QWidget>
#include "vpvl2/IKeyframe.h"

class MotionBaseModel;
class TimelineHeaderView;
class TimelineTreeView;

class QLabel;
class QPushButton;
class QSettings;
class QSpinBox;
class QTreeView;

class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    static const int kFrameIndexColumnMax = 2592000; /* 30frame * 3600sec * 24h */

    explicit TimelineWidget(MotionBaseModel *base,
                            bool stretchLastSection,
                            QWidget *parent = 0);
    ~TimelineWidget();

    int currentFrameIndex() const;
    int selectedFrameIndex() const;
    void setFrameIndexSpinBoxEnable(bool value);

    TimelineTreeView *treeView() const { return m_treeView; }

public slots:
    void setCurrentTimeIndex(const vpvl2::IKeyframe::TimeIndex &timeIndex);
    void setCurrentTimeIndex(int timeIndex);

signals:
    void motionDidSeek(const vpvl2::IKeyframe::TimeIndex &column, bool forceCameraUpdate);

private slots:
    void retranslate();
    void setCurrentTimeIndexBySpinBox();
    void setCurrentTimeIndexAndExpandBySpinBox();
    void setCurrentTimeIndexAndSelect(int frameIndex);
    void setCurrentTimeIndex(const QModelIndex &index);
    void adjustFrameColumnSize(int value);

private:
    TimelineTreeView *m_treeView;
    TimelineHeaderView *m_headerView;
    QLabel *m_label;
    QPushButton *m_button;
    QSettings *m_settings;
    QSpinBox *m_spinBox;
    QModelIndex m_index;

    Q_DISABLE_COPY(TimelineWidget)
};

#endif // TIMLINEWIDGET_H
