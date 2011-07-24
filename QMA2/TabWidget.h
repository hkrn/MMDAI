#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtGui/QTabWidget>

namespace Ui {
    class TabWidget;
}

class CameraPerspectiveWidget;
class FaceWidget;
class QSettings;

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QSettings *settings, QWidget *parent = 0);
    ~TabWidget();

    CameraPerspectiveWidget *cameraPerspectiveWidget();
    FaceWidget *faceWidget();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::TabWidget *ui;
    QSettings *m_settings;
};

#endif // TABWIDGET_H
