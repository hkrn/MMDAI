#ifndef FACEWIDGET_H
#define FACEWIDGET_H

#include <QtGui/QWidget>

namespace vpvl { class PMDModel; }

class QComboBox;
class QSlider;

class FaceWidget : public QWidget
{
    Q_OBJECT

public:
    static const int kSliderMaximumValue = 100;

    explicit FaceWidget(QWidget *parent = 0);

public slots:
    void setModel(vpvl::PMDModel *model);
    void setEyeWeight(int value);
    void setLipWeight(int value);
    void setEyeblowWeight(int value);
    void setOtherWeight(int value);

private:
    void setFaceWeight(const QString &name, int value);
    QSlider *createSlider();

    QComboBox *m_eyes;
    QComboBox *m_lips;
    QComboBox *m_eyeblows;
    QComboBox *m_others;
    vpvl::PMDModel *m_model;
};

#endif // FACEWIDGET_H
