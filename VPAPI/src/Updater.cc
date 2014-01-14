#include "Updater.h"

#ifdef Q_OS_WIN32
#include <QApplication>
#include <winsparkle.h>
#define VPVM_UPDATER_AVAILABLE true
#else
#define win_sparkle_init()
#define win_sparkle_cleanup()
#define win_sparkle_set_app_details(company_name, app_name, app_version)
#define win_sparkle_check_update_with_ui()
#define VPVM_UPDATER_AVAILABLE false
#endif

#ifndef Q_OS_MACX

Updater::Updater(QObject *parent)
    : QObject(parent)
{
    win_sparkle_init();
    win_sparkle_set_app_details(qApp->organizationName().toStdWString().c_str(),
                                qApp->applicationName().toStdWString().c_str(),
                                qApp->applicationVersion().toStdWString().c_str());
}

Updater::~Updater()
{
    win_sparkle_cleanup();
}

bool Updater::isAvailable() const
{
    return VPVM_UPDATER_AVAILABLE;
}

void Updater::checkForUpdate()
{
    win_sparkle_check_update_with_ui();
}

#endif
