#ifndef ASSETWIDGET_H
#define ASSETWIDGET_H

#include <QtGui/QWidget>

namespace vpvl {
class Asset;
class PMDModel;
}

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;

class AssetWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AssetWidget(QWidget *parent = 0);
    ~AssetWidget();

public slots:
    void addAsset(vpvl::Asset *asset);
    void removeAsset(vpvl::Asset *asset);
    void addModel(vpvl::PMDModel *model);
    void removeModel(vpvl::PMDModel *model);
    void retranslate();

signals:
    void assetDidRemove(vpvl::Asset *asset);

private slots:
    void removeAsset();
    void changeCurrentAsset(int index);
    void changeCurrentAsset(vpvl::Asset *asset);
    void changeCurrentModel(int index);
    void changeParentBone(int index);
    void updatePositionX(double value);
    void updatePositionY(double value);
    void updatePositionZ(double value);
    void updateRotationX(double value);
    void updateRotationY(double value);
    void updateRotationZ(double value);
    void updateScaleFactor(double value);
    void updateOpacity(double value);

private:
    void setEnable(bool value);

    QComboBox *m_assetComboBox;
    QComboBox *m_modelComboBox;
    QComboBox *m_modelBonesComboBox;
    QPushButton *m_removeButton;
    QDoubleSpinBox *m_px;
    QDoubleSpinBox *m_py;
    QDoubleSpinBox *m_pz;
    QDoubleSpinBox *m_rx;
    QDoubleSpinBox *m_ry;
    QDoubleSpinBox *m_rz;
    QDoubleSpinBox *m_scale;
    QDoubleSpinBox *m_opacity;
    QLabel *m_positionLabel;
    QLabel *m_rotationLabel;
    QLabel *m_scaleLabel;
    QLabel *m_opacityLabel;
    QList<vpvl::Asset *> m_assets;
    QList<vpvl::PMDModel *> m_models;
    vpvl::Asset *m_currentAsset;
    vpvl::PMDModel *m_currentModel;
};

#endif // ASSETWIDGET_H
