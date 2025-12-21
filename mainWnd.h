#ifndef _mainwnd_h_
#define _mainwnd_h_

#include "common.h"
#include "simWorker.h"

#include <QMainWindow>

class QGraphicsView;
class QStatusBar;
class QAction;
class QLabel;

class mainWnd : public QMainWindow
{
  Q_OBJECT

public:
  mainWnd(pctxT, QWidget* parent = nullptr);
  ~mainWnd();

  void readDataFile(std::string);

public slots:
  void onExit();

  void onSimConfig();
  void onReadSystem();
  void onGenSystem();
  void onRunSimulation();
  void onStopSimulation();
  void updateStatusBar(uint64_t); 

private:
  void setupUI();
  void setupActions();
  void setupMenus();


  void cleanUpSystem();

  double_t solve2ellipticIntegral(double_t);

  QAction* m_fileExit;
  QAction* m_simConfig;
  QAction* m_simBuild;
  QAction* m_simGenerate;
  QAction* m_simRun;

  std::string    m_dataFile = "";

  ctxT           m_context;
  psystemT       m_system = nullptr;
  QGraphicsView* m_graphicsview=nullptr;

  QStatusBar*    m_statusbar = nullptr;
  QLabel*        m_status = nullptr;
  QLabel*        m_time = nullptr;
  QThread*       m_simWorkerThread = nullptr;
  simWorker*     m_simWorker = nullptr;
  std::vector<orbitalPropT> m_orbitalProperties;
};

#endif

