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

#include "InterpolationWidget.h"
#include "TimelineTabWidget.h"
#include "BoneMotionModel.h"

#include <QtGui/QtGui>

InterpolationGraphWidget::InterpolationGraphWidget(BoneMotionModel *bmm, QWidget *parent)
    : QWidget(parent),
      m_boneMotionModel(bmm),
      m_type(kBone),
      m_index(0),
      m_p1Clicked(false),
      m_p2Clicked(false)
{
    setDefault(m_boneIP.x);
    setDefault(m_boneIP.y);
    setDefault(m_boneIP.z);
    setDefault(m_boneIP.rotation);
    setDefault(m_cameraIP.x);
    setDefault(m_cameraIP.y);
    setDefault(m_cameraIP.z);
    setDefault(m_cameraIP.rotation);
    setDefault(m_cameraIP.fovy);
    setDefault(m_cameraIP.distance);
    int max = kMax + 1;
    setMinimumSize(max, max);
    setMaximumSize(max, max);
}

InterpolationGraphWidget::~InterpolationGraphWidget()
{
}

void InterpolationGraphWidget::setBoneKeyFrames(const QList<BoneMotionModel::KeyFramePtr> &frames)
{
    vpvl::BoneKeyFrame *frame = frames.last().data();
    vpvl::QuadWord value(0.0f, 0.0f, 0.0f, 0.0f);
    frame->getInterpolationParameter(vpvl::BoneKeyFrame::kX, m_boneIP.x);
    frame->getInterpolationParameter(vpvl::BoneKeyFrame::kY, m_boneIP.y);
    frame->getInterpolationParameter(vpvl::BoneKeyFrame::kZ, m_boneIP.z);
    frame->getInterpolationParameter(vpvl::BoneKeyFrame::kRotation, m_boneIP.rotation);
    updateValues(true);
}

void InterpolationGraphWidget::setX1(int value)
{
    m_p1.setX(value);
    updateValues(false);
    emit x1ValueDidChange(value);
}

void InterpolationGraphWidget::setX2(int value)
{
    m_p2.setX(value);
    updateValues(false);
    emit x2ValueDidChange(value);
}

void InterpolationGraphWidget::setY1(int value)
{
    m_p1.setY(value);
    updateValues(false);
    emit y1ValueDidChange(value);
}

void InterpolationGraphWidget::setY2(int value)
{
    m_p2.setY(value);
    updateValues(false);
    emit y2ValueDidChange(value);
}

void InterpolationGraphWidget::mousePressEvent(QMouseEvent *event)
{
    int width = kCircleWidth, half = width / 2;
    QPoint p1 = m_p1, p2 = m_p2;
    p1.setY(127 - p1.y());
    p2.setY(qAbs(p2.y() - 127));
    QRect rect1(p1.x() - half, p1.y() - half, width, width);
    if (rect1.contains(event->pos())) {
        m_p1Clicked = true;
        return;
    }
    QRect rect2(p2.x() - half, p2.y() - half, width, width);
    if (rect2.contains(event->pos())) {
        m_p2Clicked = true;
        return;
    }
}

void InterpolationGraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    const int min = kMin, max = kMax;
    if (m_p1Clicked) {
        p.setX(qBound(min, p.x(), max));
        p.setY(qBound(min, max - p.y(), max));
        m_p1 = p;
        emit x1ValueDidChange(p.x());
        emit y1ValueDidChange(p.y());
    }
    else if (m_p2Clicked) {
        p.setX(qBound(min, p.x(), max));
        p.setY(qBound(min, qAbs(p.y() - max), max));
        m_p2 = p;
        emit x2ValueDidChange(p.x());
        emit y2ValueDidChange(p.y());
    }
    update();
}

void InterpolationGraphWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    m_p1Clicked = false;
    m_p2Clicked = false;
}

void InterpolationGraphWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    QPainterPath path;
    QPoint p1 = m_p1, p2 = m_p2;
    p1.setY(127 - p1.y());
    p2.setY(qAbs(p2.y() - 127));
    path.moveTo(QPoint(0, 127));
    path.cubicTo(p1, p2, QPoint(127, 0));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);
    painter.drawPath(path);
    painter.setBrush(Qt::red);
    painter.setPen(Qt::NoPen);
    int half = kCircleWidth / 2;
    painter.drawEllipse(p1.x() - half, p1.y() - half, kCircleWidth, kCircleWidth);
    painter.drawEllipse(p2.x() - half, p2.y() - half, kCircleWidth, kCircleWidth);
}

void InterpolationGraphWidget::setIndex(int value)
{
    m_index = value;
    updateValues(true);
}

void InterpolationGraphWidget::updateValues(bool import)
{
    switch (m_type) {
    case kBone:
        switch (m_index) {
        case 0:
            setValue(m_boneIP.x, import);
            break;
        case 1:
            setValue(m_boneIP.y, import);
            break;
        case 2:
            setValue(m_boneIP.z, import);
            break;
        case 3:
            setValue(m_boneIP.rotation, import);
            break;
        default:
            qWarning("Out of bone combo index: %d", m_index);
            break;
        }
        m_boneMotionModel->setInterpolationParameter(m_boneIP);
        break;
    case kCamera:
        switch (m_index) {
        case 0:
            setValue(m_cameraIP.x, import);
            break;
        case 1:
            setValue(m_cameraIP.y, import);
            break;
        case 2:
            setValue(m_cameraIP.z, import);
            break;
        case 3:
            setValue(m_cameraIP.rotation, import);
            break;
        case 4:
            setValue(m_cameraIP.distance, import);
            break;
        case 5:
            setValue(m_cameraIP.fovy, import);
            break;
        default:
            qWarning("Out of camera combo index: %d", m_index);
            break;
        }
        break;
    }
    update();
}

void InterpolationGraphWidget::setValue(vpvl::QuadWord &q, bool import)
{
    if (import) {
        m_p1.setX(q.x());
        m_p1.setY(q.y());
        m_p2.setX(q.z());
        m_p2.setY(q.w());
        emit x1ValueDidChange(q.x());
        emit y1ValueDidChange(q.y());
        emit x2ValueDidChange(q.z());
        emit y2ValueDidChange(q.w());
    }
    else {
        q.setValue(m_p1.x(), m_p1.y(), m_p2.x(), m_p2.y());
    }
}

void InterpolationGraphWidget::setDefault(vpvl::QuadWord &q)
{
    q.setValue(20, 20, 107, 107);
}

InterpolationWidget::InterpolationWidget(BoneMotionModel *bmm, QWidget *parent)
    : QWidget(parent)
{

    m_comboBox = new QComboBox();
    m_graphWidget = new InterpolationGraphWidget(bmm);
    connect(bmm, SIGNAL(boneFramesDidSelect(QList<BoneMotionModel::KeyFramePtr>)),
            m_graphWidget, SLOT(setBoneKeyFrames(QList<BoneMotionModel::KeyFramePtr>)));
    connect(m_comboBox, SIGNAL(currentIndexChanged(int)), m_graphWidget, SLOT(setIndex(int)));
    QHBoxLayout *c = new QHBoxLayout();
    QPushButton *button = new QPushButton(tr("Reset"));
    connect(button, SIGNAL(clicked()), this, SLOT(resetInterpolation()));
    c->addWidget(m_comboBox);
    c->addWidget(button);
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(new QLabel("X1"), 0, 0);
    gridLayout->addWidget(createSpinBox(20, SIGNAL(x1ValueDidChange(int)), SLOT(setX1(int))), 0, 1);
    gridLayout->addWidget(new QLabel("X2"), 1, 0);
    gridLayout->addWidget(createSpinBox(107, SIGNAL(x2ValueDidChange(int)), SLOT(setX2(int))), 1, 1);
    gridLayout->addWidget(new QLabel("Y1"), 2, 0);
    gridLayout->addWidget(createSpinBox(20, SIGNAL(y1ValueDidChange(int)), SLOT(setY1(int))), 2, 1);
    gridLayout->addWidget(new QLabel("Y2"), 3, 0);
    gridLayout->addWidget(createSpinBox(107, SIGNAL(y2ValueDidChange(int)), SLOT(setY2(int))), 3, 1);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(c);
    QHBoxLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(m_graphWidget);
    subLayout->addLayout(gridLayout);
    subLayout->setAlignment(gridLayout, Qt::AlignCenter);
    mainLayout->addLayout(subLayout);
    //mainLayout->addWidget(m_graphWidget, Qt::AlignVCenter);
    //mainLayout->setAlignment(gridLayout, Qt::AlignCenter);
    setLayout(mainLayout);
    setEnabled(false);
}

InterpolationWidget::~InterpolationWidget()
{
}

void InterpolationWidget::setMode(int mode)
{
    bool enabled = true;
    m_comboBox->clear();
    if (mode == TimelineTabWidget::kBone) {
        m_comboBox->addItem(tr("X axis"));
        m_comboBox->addItem(tr("Y axis"));
        m_comboBox->addItem(tr("Z axis"));
        m_comboBox->addItem(tr("Rotation"));
        m_graphWidget->setType(InterpolationGraphWidget::kBone);
    }
    else if (mode == TimelineTabWidget::kScene) {
        m_comboBox->addItem(tr("X axis"));
        m_comboBox->addItem(tr("Y axis"));
        m_comboBox->addItem(tr("Z axis"));
        m_comboBox->addItem(tr("Rotation"));
        m_comboBox->addItem(tr("Fovy"));
        m_comboBox->addItem(tr("Distance"));
        m_graphWidget->setType(InterpolationGraphWidget::kCamera);
    }
    else {
        enabled = false;
    }
    setEnabled(enabled);
}

void InterpolationWidget::disable()
{
    resetInterpolation();
    setEnabled(false);
}

void InterpolationWidget::resetInterpolation()
{
    m_graphWidget->setX1(20);
    m_graphWidget->setY1(20);
    m_graphWidget->setX2(107);
    m_graphWidget->setY2(107);
}

QSpinBox *InterpolationWidget::createSpinBox(int defaultValue,
                                             const char *signal,
                                             const char *slot)
{
    QSpinBox *spinBox = new QSpinBox();
    spinBox->setRange(0, 127);
    connect(spinBox, SIGNAL(valueChanged(int)), m_graphWidget, slot);
    connect(m_graphWidget, signal, spinBox, SLOT(setValue(int)));
    spinBox->setValue(defaultValue);
    return spinBox;
}
