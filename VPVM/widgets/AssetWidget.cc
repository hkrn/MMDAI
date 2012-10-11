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

#include "common/SceneLoader.h"
#include "common/util.h"
#include "widgets/AssetWidget.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

/* lupdate cannot parse tr() syntax correctly */

namespace vpvm
{

using namespace vpvl2;

AssetWidget::AssetWidget(QWidget *parent)
    : QWidget(parent),
      m_assetGroup(new QGroupBox()),
      m_assignGroup(new QGroupBox()),
      m_positionGroup(new QGroupBox()),
      m_rotationGroup(new QGroupBox()),
      m_assetComboBox(new QComboBox()),
      m_modelComboBox(new QComboBox()),
      m_modelBonesComboBox(new QComboBox()),
      m_removeButton(new QPushButton()),
      m_px(new QDoubleSpinBox()),
      m_py(new QDoubleSpinBox()),
      m_pz(new QDoubleSpinBox()),
      m_rx(new QDoubleSpinBox()),
      m_ry(new QDoubleSpinBox()),
      m_rz(new QDoubleSpinBox()),
      m_scale(new QDoubleSpinBox()),
      m_opacity(new QDoubleSpinBox()),
      m_scaleLabel(new QLabel()),
      m_opacityLabel(new QLabel()),
      m_currentAssetRef(0),
      m_currentModelRef(0)
{
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    QScopedPointer<QHBoxLayout> subLayout(new QHBoxLayout());
    /* アクセサリ選択 */
    connect(m_assetComboBox.data(), SIGNAL(currentIndexChanged(int)), this, SLOT(changeCurrentAsset(int)));
    subLayout->addWidget(m_assetComboBox.data(), 1);
    /* アクセサリ削除 */
    connect(m_removeButton.data(), SIGNAL(clicked()), this, SLOT(removeAsset()));
    subLayout->addWidget(m_removeButton.data());
    m_assetGroup->setLayout(subLayout.take());
    mainLayout->addWidget(m_assetGroup.data());
    subLayout.reset(new QHBoxLayout());
    /* 所属先のモデル選択 */
    m_modelComboBox->addItem("");
    connect(m_modelComboBox.data(), SIGNAL(currentIndexChanged(int)), this, SLOT(changeCurrentModel(int)));
    subLayout->addWidget(m_modelComboBox.data());
    /* 所属先のモデルのボーン選択 */
    connect(m_modelBonesComboBox.data(), SIGNAL(currentIndexChanged(int)), this, SLOT(changeParentBone(int)));
    subLayout->addWidget(m_modelBonesComboBox.data());
    m_assignGroup->setLayout(subLayout.take());
    mainLayout->addWidget(m_assignGroup.data());
    /* 位置(X,Y,Z) */
    const Scalar &zfar = 10000;
    QScopedPointer<QFormLayout> formLayout(new QFormLayout());
    m_px->setRange(-zfar, zfar);
    connect(m_px.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionX(double)));
    formLayout->addRow("X", m_px.data());
    m_py->setRange(-zfar, zfar);
    connect(m_py.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionY(double)));
    formLayout->addRow("Y", m_py.data());
    m_pz->setRange(-zfar, zfar);
    connect(m_pz.data(), SIGNAL(valueChanged(double)), this, SLOT(updatePositionZ(double)));
    formLayout->addRow("Z", m_pz.data());
    m_positionGroup->setLayout(formLayout.take());
    /* 回転(X,Y,Z) */
    formLayout.reset(new QFormLayout());
    m_rx->setRange(-180.0, 180.0);
    m_rx->setSingleStep(0.1);
    connect(m_rx.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationX(double)));
    formLayout->addRow("X", m_rx.data());
    m_ry->setSingleStep(0.1);
    m_ry->setRange(-180.0, 180.0);
    connect(m_ry.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationY(double)));
    formLayout->addRow("Y", m_ry.data());
    m_rz->setSingleStep(0.1);
    m_rz->setRange(-180.0, 180.0);
    connect(m_rz.data(), SIGNAL(valueChanged(double)), this, SLOT(updateRotationZ(double)));
    formLayout->addRow("Z", m_rz.data());
    m_rotationGroup->setLayout(formLayout.take());
    subLayout.reset(new QHBoxLayout());
    subLayout->addWidget(m_positionGroup.data());
    subLayout->addWidget(m_rotationGroup.data());
    mainLayout->addLayout(subLayout.take());
    subLayout.reset(new QHBoxLayout());
    /* 拡大率 */
    subLayout->addWidget(m_scaleLabel.data());
    m_scale->setSingleStep(0.1);
    m_scale->setRange(0.01, 10000.0);
    connect(m_scale.data(), SIGNAL(valueChanged(double)), this, SLOT(updateScaleFactor(double)));
    subLayout->addWidget(m_scale.data());
    /* 不透明度 */
    subLayout->addWidget(m_opacityLabel.data());
    m_opacity->setSingleStep(0.01);
    m_opacity->setRange(0.0, 1.0);
    connect(m_opacity.data(), SIGNAL(valueChanged(double)), this, SLOT(updateOpacity(double)));
    subLayout->addWidget(m_opacity.data());
    mainLayout->addLayout(subLayout.take());
    mainLayout->addStretch();
    retranslate();
    setLayout(mainLayout.take());
    setEnable(false);
}

AssetWidget::~AssetWidget()
{
}

void AssetWidget::retranslate()
{
    m_assetGroup->setTitle(vpvm::AssetWidget::tr("Asset"));
    m_assignGroup->setTitle(vpvm::AssetWidget::tr("Assign"));
    m_positionGroup->setTitle(vpvm::AssetWidget::tr("Position"));
    m_rotationGroup->setTitle(vpvm::AssetWidget::tr("Rotation"));
    m_removeButton->setText(vpvm::AssetWidget::tr("Remove"));
    m_scaleLabel->setText(vpvm::AssetWidget::tr("Scale"));
    m_opacityLabel->setText(vpvm::AssetWidget::tr("Opacity"));
    m_modelComboBox->setItemText(0, vpvm::AssetWidget::tr("Ground"));
}

void AssetWidget::addAsset(IModel *asset)
{
    /* アセットが追加されたらそのアセットが有効になるようにする。また、追加されたら表示を常に有効にする */
    m_assets.append(asset);
    m_assetComboBox->addItem(toQStringFromModel(asset));
    m_assetComboBox->setCurrentIndex(m_assetComboBox->count() - 1);
    changeCurrentAsset(asset);
    setEnable(true);
}

void AssetWidget::removeAsset(IModel *asset)
{
    int index = m_assets.indexOf(asset);
    if (index >= 0) {
        /* 該当するアセットが見つかったら表示項目から削除し、実際にアセットを削除。アセットが空なら表示を無効にしておく */
        IModel *asset = m_assets[index];
        m_assets.removeAt(index);
        m_assetComboBox->removeItem(index);
        if (m_assets.count() == 0)
            setEnable(false);
        emit assetDidRemove(asset);
    }
}

void AssetWidget::addModel(IModel *model)
{
    /*
     * アセットがモデルの特定ボーンに対して選択できるようにするための処理
     * モデルは SceneLoader が管理するのでポインタのみ。解放してはいけない
     */
    m_models.append(model);
    m_modelComboBox->addItem(toQStringFromModel(model));
}

void AssetWidget::removeModel(IModel *model)
{
    int index = modelIndexOf(model);
    if (index >= 0) {
        /* モデルが見つかればモデルとそのボーンリストを表示上から削除する。実際にモデルを削除をしない */
        m_models.removeAt(index - 1);
        m_modelComboBox->removeItem(index);
        m_modelComboBox->setCurrentIndex(0);
        m_modelBonesComboBox->clear();
    }
}

void AssetWidget::removeAsset()
{
    removeAsset(m_currentAssetRef);
}

void AssetWidget::changeCurrentAsset(int index)
{
    if (index >= 0)
        changeCurrentAsset(m_assets[index]);
}

void AssetWidget::changeCurrentAsset(IModel *asset)
{
    /* 現在のアセットの情報を更新する。回転の値はラジアン値から度数に変換しておく */
    const Vector3 &position = asset->position();
    /* setAssetProperty からも呼ばれるので、シグナル発行前に選択したアセットと同じでないことを確認する */
    bool isAssetChanged = false;
    if (m_currentAssetRef != asset) {
        m_currentAssetRef = asset;
        int index = m_assets.indexOf(asset);
        if (index >= 0)
            m_assetComboBox->setCurrentIndex(index);
        isAssetChanged = true;
    }
    m_px->setValue(position.x());
    m_py->setValue(position.y());
    m_pz->setValue(position.z());
    const Quaternion &rotation = asset->rotation();
    m_rx->setValue(degree(rotation.x()));
    m_ry->setValue(degree(rotation.y()));
    m_rz->setValue(degree(rotation.z()));
    m_scale->setValue(asset->scaleFactor());
    m_opacity->setValue(asset->opacity());
    if (isAssetChanged) {
        /* コンボボックスの更新によるシグナル発行でボーン情報が更新されてしまうため、事前にボーンを保存して再設定する */
        IModel *model = asset->parentModel();
        updateModelBoneComboBox(model);
        int index = modelIndexOf(model);
        m_modelComboBox->setCurrentIndex(index >= 0 ? index : 0);
        IBone *bone = asset->parentBone();
        if (bone) {
            const QString &name = toQStringFromBone(bone);
            index = m_modelBonesComboBox->findText(name);
            if (index >= 0)
                m_modelBonesComboBox->setCurrentIndex(index);
        }
        emit assetDidSelect(asset);
    }
}

void AssetWidget::changeCurrentModel(int index)
{
    if (index > 0) {
        /* モデルのボーンリストを一旦空にして対象のモデルのボーンリストに更新しておく */
        IModel *model = m_models[index - 1];
        m_currentModelRef = model;
        m_currentAssetRef->setParentModel(model);
        updateModelBoneComboBox(model);
    }
    else if (m_currentAssetRef) {
        /* 「地面」用。こちらは全くボーンを持たないのでボーンリストを削除した上でアセットの親ボーンを無効にしておく */
        m_modelBonesComboBox->clear();
        m_currentAssetRef->setParentModel(0);
        m_currentAssetRef->setParentBone(0);
    }
}

void AssetWidget::changeParentBone(int index)
{
    if (index >= 0 && m_currentModelRef) {
        Array<IBone *> bones;
        m_currentModelRef->getBoneRefs(bones);
        IBone *bone = bones.at(index);
        m_currentAssetRef->setParentBone(bone);
    }
    else {
        m_currentAssetRef->setParentBone(0);
    }
}

void AssetWidget::updatePositionX(double value)
{
    if (m_currentAssetRef) {
        Vector3 position = m_currentAssetRef->position();
        position.setX(value);
        m_currentAssetRef->setPosition(position);
    }
}

void AssetWidget::updatePositionY(double value)
{
    if (m_currentAssetRef) {
        Vector3 position = m_currentAssetRef->position();
        position.setY(value);
        m_currentAssetRef->setPosition(position);
    }
}

void AssetWidget::updatePositionZ(double value)
{
    if (m_currentAssetRef) {
        Vector3 position = m_currentAssetRef->position();
        position.setZ(value);
        m_currentAssetRef->setPosition(position);
    }
}

void AssetWidget::updateRotationX(double value)
{
    if (m_currentAssetRef) {
        Quaternion rotation = m_currentAssetRef->rotation();
        rotation.setRotation(Vector3(1, 0, 0), radian(value));
        m_currentAssetRef->setRotation(rotation);
    }
}

void AssetWidget::updateRotationY(double value)
{
    if (m_currentAssetRef) {
        Quaternion rotation = m_currentAssetRef->rotation();
        rotation.setRotation(Vector3(0, 1, 0), radian(value));
        m_currentAssetRef->setRotation(rotation);
    }
}

void AssetWidget::updateRotationZ(double value)
{
    if (m_currentAssetRef) {
        Quaternion rotation = m_currentAssetRef->rotation();
        rotation.setRotation(Vector3(0, 0, 1), radian(value));
        m_currentAssetRef->setRotation(rotation);
    }
}

void AssetWidget::updateScaleFactor(double value)
{
    if (m_currentAssetRef)
        m_currentAssetRef->setScaleFactor(static_cast<float>(value));
}

void AssetWidget::updateOpacity(double value)
{
    if (m_currentAssetRef)
        m_currentAssetRef->setOpacity(static_cast<float>(value));
}

void AssetWidget::setAssetProperties(IModel *asset, SceneLoader *loader)
{
    asset->setPosition(loader->assetPosition(asset));
    asset->setRotation(loader->assetRotation(asset));
    asset->setScaleFactor(loader->assetScaleFactor(asset));
    asset->setOpacity(loader->assetOpacity(asset));
    asset->setParentModel(loader->assetParentModel(asset));
    asset->setParentBone(loader->assetParentBone(asset));
    changeCurrentAsset(asset);
}

void AssetWidget::setEnable(bool value)
{
    m_removeButton->setEnabled(value);
    m_modelComboBox->setEnabled(value);
    m_modelBonesComboBox->setEnabled(value);
    m_px->setEnabled(value);
    m_py->setEnabled(value);
    m_pz->setEnabled(value);
    m_rx->setEnabled(value);
    m_ry->setEnabled(value);
    m_rz->setEnabled(value);
    m_scale->setEnabled(value);
    m_opacity->setEnabled(value);
}

void AssetWidget::updateModelBoneComboBox(IModel *model)
{
    /* changeParentBone が呼ばれてしまうので一時的に signal を切る */
    disconnect(m_modelBonesComboBox.data(), SIGNAL(currentIndexChanged(int)), this, SLOT(changeParentBone(int)));
    m_modelBonesComboBox->clear();
    if (model) {
        Array<IBone *> bones;
        model->getBoneRefs(bones);
        const int nbones = bones.count();
        for (int i = 0; i < nbones; i++) {
            IBone *bone = bones[i];
            m_modelBonesComboBox->addItem(toQStringFromBone(bone), i);
        }
    }
    connect(m_modelBonesComboBox.data(), SIGNAL(currentIndexChanged(int)), this, SLOT(changeParentBone(int)));
}

int AssetWidget::modelIndexOf(IModel *model)
{
    int index = m_models.indexOf(model);
    /* 最初の要素が地面で、特別枠なため削除してはいけない。そのためインデックスをひとつかさ上げする */
    if (index >= 0)
        index += 1;
    return index;
}

} /* namespace vpvm */
