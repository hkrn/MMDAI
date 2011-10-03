#include "AssetWidget.h"

#include "util.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

AssetWidget::AssetWidget(QWidget *parent) :
    QWidget(parent),
    m_assetComboBox(0)
{
    QLabel *label = 0;
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *subLayout = new QHBoxLayout();
    m_assetComboBox = new QComboBox();
    connect(m_assetComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCurrentAsset(int)));
    subLayout->addWidget(m_assetComboBox, 1);
    m_removeButton = new QPushButton(tr("Remove"));
    connect(m_removeButton, SIGNAL(clicked()), this, SLOT(removeAsset()));
    subLayout->addWidget(m_removeButton);
    mainLayout->addLayout(subLayout);
    subLayout = new QHBoxLayout();
    QComboBox *combo = new QComboBox();
    subLayout->addWidget(combo);
    combo = new QComboBox();
    subLayout->addWidget(combo);
    mainLayout->addLayout(subLayout);
    int index = 0;
    QGridLayout *formLayout = new QGridLayout();
    label = new QLabel(tr("Position"));
    formLayout->addWidget(label, index, 0);
    m_px = new QDoubleSpinBox();
    m_px->setRange(-10000.0, 10000.0);
    connect(m_px, SIGNAL(valueChanged(double)), this, SLOT(updatePositionX(double)));
    formLayout->addWidget(m_px, index, 1);
    m_py = new QDoubleSpinBox();
    m_py->setRange(-10000.0, 10000.0);
    connect(m_py, SIGNAL(valueChanged(double)), this, SLOT(updatePositionY(double)));
    formLayout->addWidget(m_py, index, 2);
    m_pz = new QDoubleSpinBox();
    m_pz->setRange(-10000.0, 10000.0);
    connect(m_pz, SIGNAL(valueChanged(double)), this, SLOT(updatePositionZ(double)));
    formLayout->addWidget(m_pz, index, 3);
    index++;
    label = new QLabel(tr("Rotation"));
    formLayout->addWidget(label, index, 0);
    m_rx = new QDoubleSpinBox();
    m_rx->setRange(-180.0, 180.0);
    m_rx->setSingleStep(0.1);
    connect(m_rx, SIGNAL(valueChanged(double)), this, SLOT(updateRotationX(double)));
    formLayout->addWidget(m_rx, index, 1);
    m_ry = new QDoubleSpinBox();
    m_ry->setSingleStep(0.1);
    m_ry->setRange(-180.0, 180.0);
    connect(m_ry, SIGNAL(valueChanged(double)), this, SLOT(updateRotationY(double)));
    formLayout->addWidget(m_ry, index, 2);
    m_rz = new QDoubleSpinBox();
    m_rz->setSingleStep(0.1);
    m_rz->setRange(-180.0, 180.0);
    connect(m_rz, SIGNAL(valueChanged(double)), this, SLOT(updateRotationZ(double)));
    formLayout->addWidget(m_rz, index, 3);
    mainLayout->addLayout(formLayout);
    subLayout = new QHBoxLayout();
    label = new QLabel(tr("Scale"));
    subLayout->addWidget(label);
    m_scale = new QDoubleSpinBox();
    m_scale->setSingleStep(0.1);
    m_scale->setRange(0.01, 10000.0);
    connect(m_scale, SIGNAL(valueChanged(double)), this, SLOT(updateScaleFactor(double)));
    subLayout->addWidget(m_scale);
    label = new QLabel(tr("Opacity"));
    subLayout->addWidget(label);
    m_opacity = new QDoubleSpinBox();
    m_opacity->setSingleStep(0.01);
    m_opacity->setRange(0.0, 1.0);
    connect(m_opacity, SIGNAL(valueChanged(double)), this, SLOT(updateOpacity(double)));
    subLayout->addWidget(m_opacity);
    mainLayout->addLayout(subLayout);
    setLayout(mainLayout);
    setEnable(false);
}

AssetWidget::~AssetWidget()
{
}

void AssetWidget::addAsset(vpvl::Asset *asset)
{
    m_assets.append(asset);
    m_assetComboBox->addItem(internal::toQString(asset));
    m_assetComboBox->setCurrentIndex(m_assetComboBox->count() - 1);
    changeCurrentAsset(asset);
    setEnable(true);
}

void AssetWidget::removeAsset(vpvl::Asset *asset)
{
    int index = m_assets.indexOf(asset);
    if (index >= 0) {
        vpvl::Asset *asset = m_assets[index];
        m_assets.removeAt(index);
        m_assetComboBox->removeItem(index);
        if (m_assets.count() == 0)
            setEnable(false);
        emit assetDidRemove(asset);
    }
}

void AssetWidget::removeAsset()
{
    removeAsset(m_currentAsset);
}

void AssetWidget::changeCurrentAsset(int index)
{
    if (index >= 0)
        changeCurrentAsset(m_assets[index]);
}

void AssetWidget::changeCurrentAsset(vpvl::Asset *asset)
{
    const vpvl::Vector3 &position = asset->position();
    m_px->setValue(position.x());
    m_py->setValue(position.y());
    m_pz->setValue(position.z());
    const vpvl::Quaternion &rotation = asset->rotation();
    m_rx->setValue(vpvl::degree(rotation.x()));
    m_ry->setValue(vpvl::degree(rotation.y()));
    m_rz->setValue(vpvl::degree(rotation.z()));
    m_scale->setValue(asset->scaleFactor());
    m_opacity->setValue(asset->opacity());
    m_currentAsset = asset;
}

void AssetWidget::updatePositionX(double value)
{
    if (m_currentAsset) {
        vpvl::Vector3 position = m_currentAsset->position();
        position.setX(value);
        m_currentAsset->setPosition(position);
    }
}

void AssetWidget::updatePositionY(double value)
{
    if (m_currentAsset) {
        vpvl::Vector3 position = m_currentAsset->position();
        position.setY(value);
        m_currentAsset->setPosition(position);
    }
}

void AssetWidget::updatePositionZ(double value)
{
    if (m_currentAsset) {
        vpvl::Vector3 position = m_currentAsset->position();
        position.setZ(value);
        m_currentAsset->setPosition(position);
    }
}

void AssetWidget::updateRotationX(double value)
{
    if (m_currentAsset) {
        vpvl::Quaternion rotation = m_currentAsset->rotation();
        rotation.setX(vpvl::radian(value));
        m_currentAsset->setRotation(rotation);
    }
}

void AssetWidget::updateRotationY(double value)
{
    if (m_currentAsset) {
        vpvl::Quaternion rotation = m_currentAsset->rotation();
        rotation.setY(vpvl::radian(value));
        m_currentAsset->setRotation(rotation);
    }
}

void AssetWidget::updateRotationZ(double value)
{
    if (m_currentAsset) {
        vpvl::Quaternion rotation = m_currentAsset->rotation();
        rotation.setZ(vpvl::radian(value));
        m_currentAsset->setRotation(rotation);
    }
}

void AssetWidget::updateScaleFactor(double value)
{
    if (m_currentAsset)
        m_currentAsset->setScaleFactor(static_cast<float>(value));
}

void AssetWidget::updateOpacity(double value)
{
    if (m_currentAsset)
        m_currentAsset->setOpacity(static_cast<float>(value));
}

void AssetWidget::setEnable(bool value)
{
    m_removeButton->setEnabled(value);
    m_px->setEnabled(value);
    m_py->setEnabled(value);
    m_pz->setEnabled(value);
    m_rx->setEnabled(value);
    m_ry->setEnabled(value);
    m_rz->setEnabled(value);
    m_scale->setEnabled(value);
    m_opacity->setEnabled(value);
}
