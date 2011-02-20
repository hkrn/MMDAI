#ifndef QMALOGGER_H
#define QMALOGGER_H

#include <QString>

class QMALogger
{
public:
  static void initialize();
  static const QString &getText();

private:
  QMALogger();
  ~QMALogger();
};

#endif // QMALOGGER_H
