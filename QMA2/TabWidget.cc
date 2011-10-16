#include "TabWidget.h"

#include "AssetWidget.h"
#include "CameraPerspectiveWidget.h"
#include "FaceMotionModel.h"
#include "FaceWidget.h"
#include "InterpolationWidget.h"

#include <QtGui/QtGui>

TabWidget::TabWidget(QSettings *settings, BoneMotionModel *bmm, FaceMotionModel *fmm, QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_asset(0),
    m_camera(0),
    m_face(0),
    m_interpolation(0)
{
    m_asset = new AssetWidget();
    m_camera = new CameraPerspectiveWidget();
    m_face = new FaceWidget(fmm);
    m_interpolation = new InterpolationWidget(bmm);
    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(m_asset, "");
    m_tabWidget->addTab(m_camera, "");
    m_tabWidget->addTab(m_face, "");
    m_tabWidget->addTab(m_interpolation, "");
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_tabWidget);
    retranslate();
    setMinimumSize(320, 270);
    setLayout(layout);
    restoreGeometry(m_settings->value("tabWidget/geometry").toByteArray());
}

TabWidget::~TabWidget()
{
}

void TabWidget::retranslate()
{
    m_tabWidget->setTabText(0, tr("Asset"));
    m_tabWidget->setTabText(1, tr("Camera"));
    m_tabWidget->setTabText(2, tr("Face"));
    m_tabWidget->setTabText(3, tr("Interpolation"));
    setWindowTitle(tr("Motion Tabs"));
}

void TabWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("tabWidget/geometry", saveGeometry());
    event->accept();
}
