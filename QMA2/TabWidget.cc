#include "TabWidget.h"
#include "CameraPerspectiveWidget.h"
#include "FaceMotionModel.h"
#include "FaceWidget.h"
#include "InterpolationWidget.h"

#include <QtGui/QtGui>

TabWidget::TabWidget(QSettings *settings, BoneMotionModel *bmm, FaceMotionModel *fmm, QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_camera(0),
    m_face(0)
{
    m_camera = new CameraPerspectiveWidget();
    m_face = new FaceWidget(fmm);
    m_interpolation = new InterpolationWidget(bmm);
    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->addTab(m_camera, tr("Camera"));
    tabWidget->addTab(m_face, tr("Face"));
    tabWidget->addTab(m_interpolation, tr("Interpolation"));
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabWidget);
    layout->setContentsMargins(10, 10, 10, 10);
    setMinimumSize(332, 265);
    setLayout(layout);
    restoreGeometry(m_settings->value("tabWidget/geometry").toByteArray());
    setWindowTitle(tr("Motion Tabs"));
}

TabWidget::~TabWidget()
{
}

void TabWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("tabWidget/geometry", saveGeometry());
    event->accept();
}
