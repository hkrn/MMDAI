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

#ifndef TRANSFORMWIDGET_H
#define TRANSFORMWIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QItemSelection>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>
#include <vpvl/Common.h>

namespace vpvl {
class Bone;
class Face;
class PMDModel;
}

namespace Ui {
class TransformWidget;
}

class BoneMotionModel;
class FaceMotionModel;
class QSettings;


class BoneListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    BoneListModel(BoneMotionModel *model);
    ~BoneListModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QList<vpvl::Bone *> bonesByIndices(const QModelIndexList &indices) const;
    QList<vpvl::Bone *> bonesFromIndices(const QModelIndexList &indices) const;
    void selectBones(const QList<vpvl::Bone *> &bones);

private slots:
    void changeModel(vpvl::PMDModel *model);

private:
    BoneMotionModel *m_model;
    QList<vpvl::Bone *> m_bones;
};

class FaceListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    FaceListModel(FaceMotionModel *model);
    ~FaceListModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QList<vpvl::Face *> facesByIndices(const QModelIndexList &indices) const;
    QList<vpvl::Face *> facesFromIndices(const QModelIndexList &indices) const;
    void selectFaces(const QList<vpvl::Face *> &faces);
    void startTransform();
    void commitTransform();
    void setWeight(float value);

private slots:
    void changeModel(vpvl::PMDModel *model);

private:
    FaceMotionModel *m_model;
    QList<vpvl::Face *> m_faces;
};

class TransformButton : public QPushButton
{
    Q_OBJECT

public:
    explicit TransformButton(QWidget *parent = 0);
    ~TransformButton();

    void setAngle(const vpvl::Vector3 &value) { m_angle = value; }
    void setBoneMotionModel(BoneMotionModel *value) { m_boneMotionModel = value; }

public slots:
    void setMode(int value);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    BoneMotionModel *m_boneMotionModel;
    vpvl::Vector3 m_angle;
    QCursor m_cursor;
    QPoint m_drag;
    QPoint m_pos;
};

class TransformWidget : public QWidget
{
    Q_OBJECT

public:
    enum ResetBoneType {
        kX,
        kY,
        kZ,
        kRotation
    };
    explicit TransformWidget(QSettings *settings,
                             BoneMotionModel *bmm,
                             FaceMotionModel *fmm,
                             QWidget *parent = 0);
    ~TransformWidget();

signals:
    void boneDidRegister(vpvl::Bone *bone);
    void faceDidRegister(vpvl::Face *face);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void setCameraPerspective(const vpvl::Vector3 &pos, const vpvl::Vector3 &angle, float fovy, float distance);
    void facesSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void bonesSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void on_faceWeightSlider_valueChanged(int value);
    void on_faceWeightSpinBox_valueChanged(double value);
    void on_faceWeightSlider_sliderPressed();
    void on_faceWeightSlider_sliderReleased();
    void on_comboBox_currentIndexChanged(int index);
    void on_registerButton_clicked();

private:

    Ui::TransformWidget *ui;
    BoneListModel *m_boneList;
    FaceListModel *m_faceList;
    QSettings *m_settings;
    QItemSelection m_selectedBones;
};

#endif // TRANSFORMWIDGET_H
