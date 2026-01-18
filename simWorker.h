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
  void updateStatusBar(QString);
  void updateGuiThread(uint64_t, prenderInfoT);


private:
  ctxT          m_context;
  psystemT      m_system;

  bool          m_bRunning = false;
  bool          m_bPaused = false;

  uint64_t      m_maxSteps;
};

#endif

