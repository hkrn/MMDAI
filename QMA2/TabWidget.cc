#include "TabWidget.h"
#include "ui_TabWidget.h"

#include <QtGui/QtGui>

TabWidget::TabWidget(QSettings *settings, QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::TabWidget),
    m_settings(settings)
{
    ui->setupUi(this);
    restoreGeometry(m_settings->value("tabWidget/geometry").toByteArray());
}

TabWidget::~TabWidget()
{
    delete ui;
}

CameraPerspectiveWidget *TabWidget::cameraPerspectiveWidget()
{
    return ui->cameraPerspective;
}

FaceWidget *TabWidget::faceWidget()
{
    return ui->face;
}

void TabWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("tabWidget/geometry", saveGeometry());
    event->accept();
}
