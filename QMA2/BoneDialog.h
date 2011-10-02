#ifndef BONEDIALOG_H
#define BONEDIALOG_H

#include <QtGui/QDialog>
#include <vpvl/PMDModel.h>

namespace Ui {
    class BoneDialog;
}

class BoneMotionModel;

class BoneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BoneDialog(BoneMotionModel *bmm, QWidget *parent = 0);
    ~BoneDialog();

private slots:
    void setPosition(const vpvl::Vector3 &pos);
    void setRotation(const vpvl::Quaternion &rot);
    void on_XPositionSpinBox_valueChanged(double value);
    void on_YPositionSpinBox_valueChanged(double value);
    void on_ZPositionSpinBox_valueChanged(double value);
    void on_XAxisSpinBox_valueChanged(double value);
    void on_YAxisSpinBox_valueChanged(double value);
    void on_ZAxisSpinBox_valueChanged(double value);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::BoneDialog *ui;
    BoneMotionModel *m_boneMotionModel;
};

#endif // BONEDIALOG_H
