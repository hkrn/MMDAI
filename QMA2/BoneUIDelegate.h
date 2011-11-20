#ifndef BONEUIDELEGATE_H
#define BONEUIDELEGATE_H

#include <QtCore/QObject>

class BoneMotionModel;
class MainWindow;

class BoneUIDelegate : public QObject
{
    Q_OBJECT

public:
    BoneUIDelegate(BoneMotionModel *bmm, MainWindow *parent);
    ~BoneUIDelegate();

private slots:
    void resetBoneX();
    void resetBoneY();
    void resetBoneZ();
    void resetBoneRotation();
    void resetAllBones();
    void openBoneDialog();

private:
    MainWindow *m_parent;
    BoneMotionModel *m_boneMotionModel;

    Q_DISABLE_COPY(BoneUIDelegate)
};

#endif // BONEUIDELEGATE_H
