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

#ifndef VPVM_INTERPOLATIONGRAPHWIDGET_H
#define VPVM_INTERPOLATIONGRAPHWIDGET_H

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QWidget>
#else
#include <QtGui/QAbstractItemView>
#include <QtGui/QWidget>
#endif
#include <vpvl2/IBoneKeyframe.h>
#include <vpvl2/ICameraKeyframe.h>

#include "models/BoneMotionModel.h"
#include "models/SceneMotionModel.h"

class QComboBox;
class QHBoxLayout;

namespace vpvm
{

using namespace vpvl2;
class BoneMotionModel;
class SceneMotionModel;

class InterpolationGraphWidget : public QWidget
{
    Q_OBJECT

public:
    static const int kCircleWidth = 8;
    static const int kMin = 0;
    static const int kMax = 127;
    enum Type {
        kBone,
        kCamera
    };

    InterpolationGraphWidget(BoneMotionModel *bmm, SceneMotionModel *smm, QWidget *parent = 0);
    ~InterpolationGraphWidget();

    void setModelIndices(const QModelIndexList &indices);
    void setType(Type value) { m_type = value; }
    void setLinearInterpolation();
    void reset();
    void save();

public slots:
    void setX1(int value);
    void setX2(int value);
    void setY1(int value);
    void setY2(int value);

signals:
    void x1ValueDidChange(int value);
    void x2ValueDidChange(int value);
    void y1ValueDidChange(int value);
    void y2ValueDidChange(int value);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void applyAll();
    void selectParameterType(int value);

private:
    void updateValues(bool import);
    void setValue(QuadWord &q, bool import);
    void setDefault(QuadWord &q);

    BoneMotionModel *m_boneMotionModelRef;
    SceneMotionModel *m_sceneMotionModelRef;
    BoneMotionModel::KeyFramePairList m_boneKeyframes;
    SceneMotionModel::CameraKeyframePairList m_cameraKeyframes;
    IBoneKeyframe::InterpolationParameter m_boneIP;
    IBoneKeyframe::InterpolationParameter m_preservedBoneIP;
    ICameraKeyframe::InterpolationParameter m_cameraIP;
    ICameraKeyframe::InterpolationParameter m_preservedCameraIP;
    QPoint m_p1;
    QPoint m_p2;
    Type m_type;
    int m_index;
    bool m_p1Clicked;
    bool m_p2Clicked;

    Q_DISABLE_COPY(InterpolationGraphWidget)
};

} /* namespace vpvm */

#endif // INTERPOLATIONWIDGET_H
