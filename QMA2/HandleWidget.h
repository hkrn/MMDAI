#ifndef HANDLEWIDGET_H
#define HANDLEWIDGET_H

#include <QtGui/QWidget>
#include <vpvl/Bone.h>

class QGraphicsScene;
class XHandle;
class YHandle;
class ZHandle;

class HandleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HandleWidget(QWidget *parent = 0);
    ~HandleWidget();

public slots:
    void setBone(vpvl::Bone *value);
    void setCameraPerspective(const btVector3 &pos, const btVector3 &angle, float fovy, float distance);

private:
    void createHandles(QGraphicsScene *scene);

    vpvl::BoneList m_bones;
    XHandle *m_xHandle;
    YHandle *m_yHandle;
    ZHandle *m_zHandle;
};

#endif // HANDLEWIDGET_H
