#ifndef QMAMODELLOADERFACTORY_H
#define QMAMODELLOADERFACTORY_H

#include <QSet>

#include "MMDAI/PMDModelLoaderFactory.h"

#include "QMAModelLoader.h"

class QMAModelLoaderFactory : public PMDModelLoaderFactory
{
public:
  QMAModelLoaderFactory();
  ~QMAModelLoaderFactory();

  PMDModelLoader *createModelLoader(const char *filename);

private:
  QSet<QMAModelLoader *> m_loaders;
};

#endif // QMAMODELLOADERFACTORY_H
