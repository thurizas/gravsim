#ifndef _mainwnd_h_
#define _mainwnd_h_

#include "common.h"
#include "simWorker.h"
#include "detailsDlg.h"

#include <QMainWindow>

//class QGraphicsView;
class TwoDGeomView;
class QGraphicsScene;
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
  //void getSystem(psystemT);

public slots:
  void onExit();

  void onSimConfig();
  void onReadSystem(std::string* inFile = nullptr);
  void onGenSystem();
  void onViewCenter();
  void onViewZoomIn();
  void onViewZoomOut();
  void onViewZoomFit();
  void onViewSystem();
  void onRunSimulation();
  void onPauseSimulation();
  void onResumeSimulation();
  void onStopSimulation();
  void updateStatusBar(uint64_t); 
  //void updateDetails();


private:
  void setupUI();
  void setupActions();
  void setupMenus();
  void updateDisplay();
  void cleanUpSystem();

  double_t solve2ellipticIntegral(double_t);
  void     clearOldData();

  QAction* m_fileExit;
  QAction* m_viewSystem;
  QAction* m_viewCenter;
  QAction* m_zoomIn;
  QAction* m_zoomOut;
  QAction* m_zoomFit;
  QAction* m_simConfig;
  QAction* m_simBuild;
  QAction* m_simGenerate;
  QAction* m_simRun;
  QAction* m_simPause;
  QAction* m_simResume;
  QAction* m_simStop;

  // state variables
  bool       m_bRunning = false;
  bool       m_bPaused = false;

  std::string    m_dataFile = "";
  std::string    m_cmdLineName = "";

  ctxT            m_context;
  psystemT        m_system = nullptr;
  TwoDGeomView*   m_graphicsview=nullptr;
  QGraphicsScene* m_graphicsScene = nullptr;
  QPoint          m_center;

  QStatusBar*    m_statusbar = nullptr;
  QLabel*        m_status = nullptr;
  QLabel*        m_time = nullptr;
  //QThread*       m_simWorkerThread = nullptr;
  //simWorker*     m_simWorker = nullptr;
  std::vector<orbitalPropT> m_orbitalProperties;
};

#endif

