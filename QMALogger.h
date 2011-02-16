#ifndef QMALOGGER_H
#define QMALOGGER_H

class QMALogger
{
public:
  ~QMALogger();

  static void initialize();
  QMALogger *getLogger();

private:
  QMALogger();
};

#endif // QMALOGGER_H
