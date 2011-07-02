#ifndef FACEWIDGET_H
#define FACEWIDGET_H

#include <QtGui/QWidget>

namespace vpvl { class PMDModel; }

class QComboBox;

class FaceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FaceWidget(QWidget *parent = 0);

public slots:
    void setModel(vpvl::PMDModel *model);

private:
    QComboBox *m_eyes;
    QComboBox *m_lips;
    QComboBox *m_eyeblows;
    QComboBox *m_others;
    vpvl::PMDModel *m_model;
};

#endif // FACEWIDGET_H
