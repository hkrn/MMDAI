#ifndef HANDLEWIDGET_H
#define HANDLEWIDGET_H

#include <QWidget>

class HandleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HandleWidget(QWidget *parent = 0);
    ~HandleWidget();

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // HANDLEWIDGET_H
