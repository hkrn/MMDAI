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

#ifndef VPVM_ASSETWIDGET_H
#define VPVM_ASSETWIDGET_H

#include <vpvl2/IModel.h>
#include <vpvl2/qt/RenderContext.h>

#include <QTextStream>
#include <QUuid>
#include <QStringListModel>
#include <QWidget>

namespace vpvl2 {
class IBone;
}

class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;

namespace vpvm
{

using namespace vpvl2;
using namespace vpvl2::qt;
class SceneLoader;

class AssetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AssetWidget(SceneLoader *loaderRef, QWidget *parent = 0);
    ~AssetWidget();

    IModelSharedPtr currentAsset() const { return m_currentAssetRef; }
    IModelSharedPtr currentModel() const { return m_currentModelRef; }

public slots:
    void addAsset(IModelSharedPtr asset);
    void removeAsset(IModelSharedPtr asset);
    void addModel(IModelSharedPtr model);
    void removeModel(IModelSharedPtr model);
    void retranslate();

signals:
    void assetDidSelect(IModelSharedPtr asset);
    void assetDidRemove(IModelSharedPtr asset);

private slots:
    void deleteCurrentAsset();
    void changeCurrentAsset(int index);
    void changeCurrentAsset(IModelSharedPtr asset);
    void changeCurrentModel(int index);
    void changeParentBone(int index);
    void updatePositionX(double value);
    void updatePositionY(double value);
    void updatePositionZ(double value);
    void updateRotation();
    void updateScaleFactor(double value);
    void updateOpacity(double value);
    void setAssetProperties(IModelSharedPtr asset);

private:
    static QDoubleSpinBox *createSpinBox(double step, double min, double max);
    void setEnable(bool value);
    void updateModelBoneComboBox(IModelSharedPtr model);
    int modelIndexOf(IModelSharedPtr model);

    SceneLoader *m_sceneLoaderRef;
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
    QList<IModelSharedPtr> m_assets;
    QList<IModelSharedPtr> m_models;
    IModelSharedPtr m_currentAssetRef;
    IModelSharedPtr m_currentModelRef;

    Q_DISABLE_COPY(AssetWidget)
};

} /* namespace vpvl2 */

#endif // ASSETWIDGET_H
