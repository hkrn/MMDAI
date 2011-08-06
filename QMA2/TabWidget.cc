#include "TabWidget.h"
#include "CameraPerspectiveWidget.h"
#include "FaceWidget.h"

#include <QtGui/QtGui>

TabWidget::TabWidget(QSettings *settings, QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_camera(0),
    m_face(0)
{
    m_camera = new CameraPerspectiveWidget(), tr("Camera");
    m_face = new FaceWidget();
    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->addTab(m_camera, tr("Camera"));
    tabWidget->addTab(m_face, tr("Face"));
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabWidget);
    layout->setContentsMargins(10, 10, 10, 10);
    setMinimumSize(332, 265);
    setLayout(layout);
    restoreGeometry(m_settings->value("tabWidget/geometry").toByteArray());
}

TabWidget::~TabWidget()
{
}

CameraPerspectiveWidget *TabWidget::cameraPerspectiveWidget()
{
    return m_camera;
}

FaceWidget *TabWidget::faceWidget()
{
    return m_face;
}

void TabWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("tabWidget/geometry", saveGeometry());
    event->accept();
}
