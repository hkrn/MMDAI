#include "SharingService.h"

#ifndef Q_OS_MACX

QStringList SharingService::availableServiceNames()
{
    return QStringList();
}

SharingService::SharingService()
{
}

SharingService::~SharingService()
{
}

void SharingService::setServiceName(const QString & /* value */)
{
}

void SharingService::showPostForm(const QImage & /* image */)
{
}

#endif
