#include "HandleWidget.h"

#include <QtGui/QtGui>

HandleWidget::HandleWidget(QWidget *parent) :
    QWidget(parent)
{
}

HandleWidget::~HandleWidget()
{
}

void HandleWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    int x = 10, y = 10, w = width() - x * 2, h = height() - y * 2, cx = w / 2 + x, cy = h / 2 + y;
    QPen pen;
    pen.setWidth(5);
    painter.setRenderHint(QPainter::Antialiasing);
    pen.setColor(Qt::green);
    painter.setPen(pen);
    painter.drawLine(cx, y, cx, h + y);
    pen.setColor(Qt::red);
    painter.setPen(pen);
    painter.drawLine(x, cy, w + x, cy);
    pen.setColor(Qt::blue);
    painter.setPen(pen);
    painter.drawEllipse(x, y, w, h);
    painter.end();
}
