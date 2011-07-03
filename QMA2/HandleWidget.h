#ifndef HANDLEWIDGET_H
#define HANDLEWIDGET_H

#include <QtGui/QWidget>

namespace vpvl { class Bone; }

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

private:
    void createHandles(QGraphicsScene *scene);

    vpvl::Bone *m_bone;
    XHandle *m_xHandle;
    YHandle *m_yHandle;
    ZHandle *m_zHandle;
};

#endif // HANDLEWIDGET_H
