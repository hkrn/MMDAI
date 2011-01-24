#include "QMAModelLoaderFactory.h"

#include <QDir>
#include <QString>

#include "QMAModelLoader.h"

QMAModelLoaderFactory::QMAModelLoaderFactory()
{
}

QMAModelLoaderFactory::~QMAModelLoaderFactory()
{
  foreach (QMAModelLoader *loader, m_loaders) {
    delete loader;
  }
}

PMDModelLoader *QMAModelLoaderFactory::createModelLoader(const char *filename)
{
  QString path = QDir(QDir::currentPath()).absoluteFilePath("AppData");
  QMAModelLoader *loader = new QMAModelLoader(path, filename);
  m_loaders.insert(loader);
  return loader;
}
