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

#ifndef VPVM_ASSETWIDGET_H
#define VPVM_ASSETWIDGET_H

#include <QtCore/QTextStream>
#include <QtCore/QUuid>
#include <QtGui/QStringListModel>
#include <QtGui/QWidget>

namespace vpvl2 {
class IBone;
class IModel;
}

class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;

namespace vpvm
{

using namespace vpvl2;
class SceneLoader;

class AssetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AssetWidget(QWidget *parent = 0);
    ~AssetWidget();

    IModel *currentAsset() const { return m_currentAssetRef; }
    IModel *currentModel() const { return m_currentModelRef; }

public slots:
    void addAsset(IModel *asset);
    void removeAsset(IModel *asset);
    void addModel(IModel *model);
    void removeModel(IModel *model);
    void retranslate();

signals:
    void assetDidSelect(IModel *asset);
    void assetDidRemove(IModel *asset);

private slots:
    void removeAsset();
    void changeCurrentAsset(int index);
    void changeCurrentAsset(IModel *asset);
    void changeCurrentModel(int index);
    void changeParentBone(int index);
    void updatePositionX(double value);
    void updatePositionY(double value);
    void updatePositionZ(double value);
    void updateRotation();
    void updateScaleFactor(double value);
    void updateOpacity(double value);
    void setAssetProperties(IModel *asset, SceneLoader *loader);

private:
    void setEnable(bool value);
    void updateModelBoneComboBox(IModel *model);
    int modelIndexOf(IModel *model);

    QScopedPointer<QGroupBox> m_assetGroup;
    QScopedPointer<QGroupBox> m_assignGroup;
    QScopedPointer<QGroupBox> m_positionGroup;
    QScopedPointer<QGroupBox> m_rotationGroup;
    QScopedPointer<QComboBox> m_assetComboBox;
    QScopedPointer<QComboBox> m_modelComboBox;
    QScopedPointer<QComboBox> m_modelBonesComboBox;
    QScopedPointer<QPushButton> m_removeButton;
    QScopedPointer<QDoubleSpinBox> m_px;
    QScopedPointer<QDoubleSpinBox> m_py;
    QScopedPointer<QDoubleSpinBox> m_pz;
    QScopedPointer<QDoubleSpinBox> m_rx;
    QScopedPointer<QDoubleSpinBox> m_ry;
    QScopedPointer<QDoubleSpinBox> m_rz;
    QScopedPointer<QDoubleSpinBox> m_scale;
    QScopedPointer<QDoubleSpinBox> m_opacity;
    QScopedPointer<QStringListModel> m_assetCompleterModel;
    QScopedPointer<QLabel> m_scaleLabel;
    QScopedPointer<QLabel> m_opacityLabel;
    QList<IModel *> m_assets;
    QList<IModel *> m_models;
    IModel *m_currentAssetRef;
    IModel *m_currentModelRef;

    Q_DISABLE_COPY(AssetWidget)
};

} /* namespace vpvl2 */

#endif // ASSETWIDGET_H
