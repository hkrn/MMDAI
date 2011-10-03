#ifndef ASSETWIDGET_H
#define ASSETWIDGET_H

#include <QtGui/QWidget>

namespace vpvl {
class Asset;
}

class QComboBox;
class QDoubleSpinBox;
class QPushButton;

class AssetWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AssetWidget(QWidget *parent = 0);
    ~AssetWidget();

signals:
    void assetDidRemove(vpvl::Asset *asset);

public slots:
    void addAsset(vpvl::Asset *asset);
    void removeAsset(vpvl::Asset *asset);

private slots:
    void removeAsset();
    void changeCurrentAsset(int index);
    void changeCurrentAsset(vpvl::Asset *asset);
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
    QPushButton *m_removeButton;
    QDoubleSpinBox *m_px;
    QDoubleSpinBox *m_py;
    QDoubleSpinBox *m_pz;
    QDoubleSpinBox *m_rx;
    QDoubleSpinBox *m_ry;
    QDoubleSpinBox *m_rz;
    QDoubleSpinBox *m_scale;
    QDoubleSpinBox *m_opacity;
    QList<vpvl::Asset *> m_assets;
    vpvl::Asset *m_currentAsset;
};

#endif // ASSETWIDGET_H
