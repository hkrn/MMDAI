#ifndef FACEWIDGET_H
#define FACEWIDGET_H

#include <QtGui/QWidget>

namespace vpvl {
class PMDModel;
class Face;
}

class FaceMotionModel;
class QComboBox;
class QSlider;

class FaceWidget : public QWidget
{
    Q_OBJECT

public:
    static const int kSliderMaximumValue = 100;

    explicit FaceWidget(FaceMotionModel *fmm, QWidget *parent = 0);

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    void setEyeWeight(int value);
    void setLipWeight(int value);
    void setEyeblowWeight(int value);
    void setOtherWeight(int value);

signals:
    void faceDidRegister(vpvl::Face *face);

private slots:
    void registerEye();
    void registerLip();
    void registerEyeblow();
    void registerOther();

private:
    void setFaceWeight(const QComboBox *comboBox, int value);
    void registerBase(const QComboBox *comboBox);
    vpvl::Face *findFace(const QString &name);
    QSlider *createSlider();

    QComboBox *m_eyes;
    QComboBox *m_lips;
    QComboBox *m_eyeblows;
    QComboBox *m_others;
    FaceMotionModel *m_faceMotionModel;
};

#endif // FACEWIDGET_H
