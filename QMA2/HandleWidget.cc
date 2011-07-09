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
          m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
          m_mode(kView),
          m_prev(0, 0),
          m_rect(x, y, w, h)
    {
        m_pen.setColor(Qt::black);
        m_pen.setWidth(15);
    }
    ~BaseHandle() {
    }

    HandleMode handleMode() const {
        return m_mode;
    }
    QRectF boundingRect() const {
        return m_rect;
    }
    void setAngle(const btVector3 &angle) {
        m_rotation.setEulerZYX(vpvl::radian(angle.z()), vpvl::radian(angle.y()), vpvl::radian(angle.x()));
    }
    void setBone(const vpvl::BoneList &bones) {
        m_bones.copyFromArray(bones);
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
        if (m_bones.size() > 0)
            moveBones(pos);
        m_prev = current;
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
        QPointF current = event->pos(), pos = current - m_prev;
        if (m_bones.size())
            moveBones(pos);
        m_prev = current;
    }
    void translateBones(const btVector3 &v) {
        uint32_t nBones = m_bones.size();
        switch (handleMode()) {
        case kView: {
            for (uint32_t i = 0; i < nBones; i++) {
                vpvl::Bone *bone = m_bones[i];
                btTransform tr(m_rotation, bone->position());
                bone->setPosition(tr * v);
            }
            break;
        }
        case kLocal: {
            for (uint32_t i = 0; i < nBones; i++) {
                vpvl::Bone *bone = m_bones[i];
                btTransform tr(bone->rotation(), bone->position());
                bone->setPosition(tr * v);
            }
            break;
        }
        case kGlobal: {
            for (uint32_t i = 0; i < nBones; i++) {
                vpvl::Bone *bone = m_bones[i];
                bone->setPosition(bone->position() + v);
            }
            break;
        }
        }
    }

    virtual void moveBones(QPointF &diff) = 0;

    QPen m_pen;

private:
    btQuaternion m_rotation;
    vpvl::BoneList m_bones;
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
    void moveBones(QPointF &diff) {
        btVector3 v(diff.x(), 0.0f, 0.0f);
        translateBones(v);
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
    void moveBones(QPointF &diff) {
        btVector3 v(0.0f, diff.y(), 0.0f);
        translateBones(v);
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
    void moveBones(QPointF &diff) {
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
}

void HandleWidget::setBone(vpvl::Bone *value)
{
    if (value) {
        m_bones.clear();
        m_bones.push_back(value);
        m_xHandle->setBone(m_bones);
        m_yHandle->setBone(m_bones);
        m_zHandle->setBone(m_bones);
    }
}

void HandleWidget::setCameraPerspective(const btVector3 & /* pos */, const btVector3 &angle, float /* fovy */, float /* distance */)
{
    m_xHandle->setAngle(angle);
    m_yHandle->setAngle(angle);
    m_zHandle->setAngle(angle);
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
