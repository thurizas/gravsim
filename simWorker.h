#ifndef _simworker_h_
#define _simworker_h_

#include "common.h"

#include <QObject>


class simWorker : public QObject
{
  Q_OBJECT

public:
  simWorker(pctxT pcontext, psystemT psystem, QObject* parent = nullptr);

public slots:
  void start();

signals:
  void tick(uint64_t);
  void calcDone();
  void finished();

private:
  ctxT          m_context;
  psystemT      m_system;
};

#endif

