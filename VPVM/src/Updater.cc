#include "Updater.h"

#ifndef Q_OS_MACX

Updater::Updater(QObject *parent)
    : QObject(parent)
{
}

Updater::~Updater()
{
}

bool Updater::isAvailable()
{
    return false;
}

void Updater::checkForUpdate()
{
    /* do nothing */
}

#endif
