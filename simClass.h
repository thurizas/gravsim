#ifndef _simthread_h_
#define _simthread_h_

#include "constants.h"
#include "common.h"


#include <QThread>

class simClass : public QThread
{
  Q_OBJECT

public:
  simClass(pctxT, QObject* parent = nullptr);

  void run();

private:
  ctxT        m_context;

};

#endif

