#include "InterpolationWidget.h"
#include "TimelineTabWidget.h"
#include "BoneMotionModel.h"

#include <QtGui/QtGui>

InterpolationGraphWidget::InterpolationGraphWidget(BoneMotionModel *bmm, QWidget *parent)
    : QWidget(parent),
      m_boneMotionModel(bmm)
{
}

InterpolationGraphWidget::~InterpolationGraphWidget()
{
}

void InterpolationGraphWidget::setX1(int value)
{
    m_p1.setX(value);
    updateValues();
    emit x1ValueDidChange(value);
}

void InterpolationGraphWidget::setX2(int value)
{
    m_p2.setX(value);
    updateValues();
    emit x2ValueDidChange(value);
}

void InterpolationGraphWidget::setY1(int value)
{
    m_p1.setY(value);
    updateValues();
    emit y1ValueDidChange(value);
}

void InterpolationGraphWidget::setY2(int value)
{
    m_p2.setY(value);
    updateValues();
    emit y2ValueDidChange(value);
}

void InterpolationGraphWidget::mousePressEvent(QMouseEvent *event)
{
    int width = kCircleWidth * 2;
    QRect rect1(rect().x() + m_p1.x(), rect().y() + m_p1.y(), width, width);
    if (rect1.contains(event->pos()))
        qDebug() << "contain m_p1";
    QRect rect2(rect().x() + m_p2.x(), rect().y() + m_p2.y(), width, width);
    if (rect2.contains(event->pos()))
        qDebug() << "contain m_p2";
}

void InterpolationGraphWidget::mouseMoveEvent(QMouseEvent * /* event */)
{
}

void InterpolationGraphWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
}

void InterpolationGraphWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    QPainterPath path;
    path.cubicTo(m_p1, m_p2, QPoint(127, 127));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);
    painter.drawPath(path);
    painter.setBrush(Qt::blue);
    painter.drawEllipse(m_p1.x(), m_p1.y(), kCircleWidth, kCircleWidth);
    painter.drawEllipse(m_p2.x(), m_p2.y(), kCircleWidth, kCircleWidth);
}

void InterpolationGraphWidget::updateValues()
{
    repaint();
}

InterpolationWidget::InterpolationWidget(BoneMotionModel *bmm, QWidget *parent)
    : QWidget(parent),
      m_comboBox(0),
      m_graphWidget(0)
{
    m_comboBox = new QComboBox();
    m_graphWidget = new InterpolationGraphWidget(bmm);
    m_graphWidget->setMinimumSize(128, 128);
    m_graphWidget->setMaximumSize(128, 128);
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *c = new QHBoxLayout();
    QPushButton *button = new QPushButton(tr("Reset"));
    connect(button, SIGNAL(clicked()), this, SLOT(resetInterpolation()));
    c->addWidget(m_comboBox);
    c->addWidget(button);
    layout->addLayout(c);
    QHBoxLayout *x = new QHBoxLayout();
    createSpinBox(x, "x1", 20, SIGNAL(x1ValueDidChange(int)), SLOT(setX1(int)));
    createSpinBox(x, "x2", 107, SIGNAL(x2ValueDidChange(int)), SLOT(setX2(int)));
    layout->addLayout(x);
    QHBoxLayout *y = new QHBoxLayout();
    createSpinBox(y, "y1", 20, SIGNAL(y1ValueDidChange(int)), SLOT(setY1(int)));
    createSpinBox(y, "y2", 107, SIGNAL(y2ValueDidChange(int)), SLOT(setY2(int)));
    layout->addLayout(y);
    layout->addWidget(m_graphWidget, Qt::AlignVCenter);
    setLayout(layout);
    setEnabled(false);
}

InterpolationWidget::~InterpolationWidget()
{
}

void InterpolationWidget::setMode(const QString &mode)
{
    bool enabled = true;
    m_comboBox->clear();
    if (mode == TimelineTabWidget::kBone) {
        m_comboBox->addItem(tr("X axis"));
        m_comboBox->addItem(tr("Y axis"));
        m_comboBox->addItem(tr("Z axis"));
        m_comboBox->addItem(tr("Rotation"));
    }
    else if (mode == TimelineTabWidget::kCamera) {
        m_comboBox->addItem(tr("X axis"));
        m_comboBox->addItem(tr("Y axis"));
        m_comboBox->addItem(tr("Z axis"));
        m_comboBox->addItem(tr("Rotation"));
        m_comboBox->addItem(tr("Distance"));
        m_comboBox->addItem(tr("Fovy"));
    }
    else {
        enabled = false;
    }
    setEnabled(enabled);
}

void InterpolationWidget::disable()
{
    resetInterpolation();
    setEnabled(false);
}

void InterpolationWidget::resetInterpolation()
{
    m_graphWidget->setX1(20);
    m_graphWidget->setY1(20);
    m_graphWidget->setX2(107);
    m_graphWidget->setY2(107);
}

void InterpolationWidget::createSpinBox(QHBoxLayout *layout,
                                        const QString &label,
                                        int defaultValue,
                                        const char *signal,
                                        const char *slot)
{
    QSpinBox *spinBox = new QSpinBox();
    spinBox->setRange(0, 127);
    layout->addWidget(new QLabel(label), Qt::AlignCenter);
    layout->addWidget(spinBox);
    connect(spinBox, SIGNAL(valueChanged(int)), m_graphWidget, slot);
    connect(m_graphWidget, signal, spinBox, SLOT(setValue(int)));
    spinBox->setValue(defaultValue);
}
