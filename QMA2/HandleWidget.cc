#include "HandleWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

class BaseHandle : public QGraphicsItem
{
public:
    enum HandleMode {
        kView,
        kLocal,
        kGlobal
    };

    BaseHandle(qreal x, qreal y, qreal w, qreal h)
        : QGraphicsItem(),
          m_bone(0),
          m_mode(kLocal),
          m_prev(0, 0),
          m_rect(x, y, w, h)
    {
        m_pen.setColor(Qt::black);
        m_pen.setWidth(15);
    }
    ~BaseHandle() {
        m_bone = 0;
    }

    HandleMode handleMode() const {
        return m_mode;
    }
    QRectF boundingRect() const {
        return m_rect;
    }
    void setBone(vpvl::Bone *value) {
        m_bone = value;
    }
    void setHandleMode(HandleMode value) {
        m_mode = value;
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) {
        m_prev = event->pos();
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
        QPointF current = event->pos(), pos = current - m_prev;
        if (m_bone)
            moveBone(m_bone, pos);
        m_prev = current;
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
        QPointF current = event->pos(), pos = current - m_prev;
        if (m_bone)
            moveBone(m_bone, pos);
        m_prev = current;
    }
    virtual void moveBone(vpvl::Bone *bone, QPointF &diff) = 0;

    QPen m_pen;

private:
    vpvl::Bone *m_bone;
    HandleMode m_mode;
    QPointF m_prev;
    QRectF m_rect;
};

class XHandle : public BaseHandle
{
public:
    XHandle(qreal x, qreal y, qreal w, qreal h)
        : BaseHandle(x, y, w, h) {
        QColor color = Qt::green;
        color.setAlphaF(0.5f);
        m_pen.setColor(color);
    }
    ~XHandle() {}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */) {
        QRectF rect = boundingRect();
        painter->setPen(m_pen);
        painter->drawLine(QLineF(rect.x(), rect.y(), rect.width(), rect.height()));
    }

protected:
    void moveBone(vpvl::Bone *bone, QPointF &diff) {
        btVector3 v(diff.x(), 0.0f, 0.0f);
        switch (handleMode()) {
        case kView: {
            break;
        }
        case kLocal: {
            btTransform tr(bone->currentRotation(), bone->currentPosition());
            bone->setCurrentPosition(tr * v);
            break;
        }
        case kGlobal: {
            bone->setCurrentPosition(bone->currentPosition() + v);
            break;
        }
        }
    }
};

class YHandle : public BaseHandle
{
public:
    YHandle(qreal x, qreal y, qreal w, qreal h)
        : BaseHandle(x, y, w, h) {
        QColor color = Qt::red;
        color.setAlphaF(0.5f);
        m_pen.setColor(color);
    }
    ~YHandle() {}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */) {
        QRectF rect = boundingRect();
        painter->setPen(m_pen);
        painter->drawLine(QLineF(rect.x(), rect.y(), rect.width(), rect.height()));
    }

protected:
    void moveBone(vpvl::Bone *bone, QPointF &diff) {
        btVector3 v(0.0f, diff.y(), 0.0f);
        switch (handleMode()) {
        case kView: {
            break;
        }
        case kLocal: {
            btTransform tr(bone->currentRotation(), bone->currentPosition());
            bone->setCurrentPosition(tr * v);
            break;
        }
        case kGlobal: {
            bone->setCurrentPosition(bone->currentPosition() + v);
            break;
        }
        }
    }
};

class ZHandle : public BaseHandle
{
public:
    ZHandle(qreal x, qreal y, qreal w, qreal h)
        : BaseHandle(x, y, w, h) {
        QColor color = Qt::blue;
        color.setAlphaF(0.5f);
        m_pen.setColor(color);
    }
    ~ZHandle() {}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem * /* option */, QWidget * /* widget */) {
        painter->setPen(m_pen);
        painter->drawEllipse(boundingRect());
    }

protected:
    void moveBone(vpvl::Bone *bone, QPointF &diff) {
        Q_UNUSED(bone);
        Q_UNUSED(diff);
    }
};

HandleWidget::HandleWidget(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    QGraphicsScene *scene = new QGraphicsScene();
    createHandles(scene);
    QGraphicsView *view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    layout->setContentsMargins(QMargins());
    layout->addWidget(view);
    setLayout(layout);
    setMinimumHeight(200);
}

HandleWidget::~HandleWidget()
{
    m_bone = 0;
}

void HandleWidget::setBone(vpvl::Bone *value)
{
    m_bone = value;
    m_xHandle->setBone(value);
    m_yHandle->setBone(value);
    m_zHandle->setBone(value);
}

void HandleWidget::createHandles(QGraphicsScene *scene)
{
    int x = 0, y = 0, w = 150, h = 150, cx = w / 2 + x, cy = h / 2 + y;
    m_xHandle = new XHandle(x, cy, w + x, cy);
    m_yHandle = new YHandle(cx, y, cx, h + y);
    m_zHandle = new ZHandle(x, y, w, h);
    scene->addItem(m_zHandle);
    scene->addItem(m_xHandle);
    scene->addItem(m_yHandle);
}
