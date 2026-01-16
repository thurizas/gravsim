#include "mainWnd.h"
#include "common.h"
#include "constants.h"

#include "2DGraphicsView.h"
#include "simWorker.h"
#include "simPropertiesDlg.h"
#include "detailsDlg.h"

#include "libprocgen\procgen.h"

#include <iostream>
#include <iomanip>

#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QThread>
#include <QMessageBox>
#include <QPainterPath>


mainWnd::mainWnd(pctxT context, QWidget* p) : QMainWindow(p), m_context(*context)
{
  setupUI();
  setupActions();
  setupMenus();

  m_graphicsScene = new QGraphicsScene;
}


mainWnd:: ~mainWnd()
{

}

/**********************************************************************************************************************
 * Function: readDataFile
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Dec 2025 (gkhuber) 
 *********************************************************************************************************************/
void mainWnd::readDataFile(std::string name)
{
  QFile inFile(name.c_str());

  if (inFile.exists())
  {
    //m_dataFile = name;
    m_cmdLineName = name;
    onReadSystem();
    m_cmdLineName.clear();

    m_statusbar->showMessage(QString("using data file %1").arg(m_dataFile.c_str()), 500);
    this->setWindowTitle(QString("Orbital Simulator - data file %1").arg(m_dataFile.c_str()));
  }
  else
  {
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::WARNING, "file %s does not exist", name.c_str());
  }
}


void mainWnd::setupUI()
{
  QMenuBar* menubar;

  if (this->objectName().isEmpty())
    this->setObjectName("mainWindow");
  this->resize(1024, 768);
  this->setWindowTitle("Orbital Simulator");

  QWidget* centralWidget = new QWidget(this);
  centralWidget->setObjectName("centralWidget");

  QHBoxLayout* hLayout = new QHBoxLayout(centralWidget);
  hLayout->setSpacing(6);
  hLayout->setContentsMargins(11, 11, 11, 11);
  hLayout->setObjectName("mainLayout");

  m_graphicsview = new TwoDGeomView();
  m_graphicsview->setObjectName("graphicsView");

  hLayout->addWidget(m_graphicsview);

  menubar = new QMenuBar(this);
  menubar->setObjectName(QString::fromUtf8("menuBar"));
  menubar->setGeometry(QRect(0, 0, 799, 21));
  this->setMenuBar(menubar);

  m_statusbar = new QStatusBar(this);
  m_statusbar->setObjectName(QString::fromUtf8("statusBar"));
  
  m_status = new QLabel(m_statusbar);
  m_status->setText("Ready                        ");
  QLabel* label1 = new QLabel(m_statusbar);
  label1->setText("Simulation time: ");
  m_time = new QLabel(m_statusbar);
  m_time->setText(QString("%1 days").arg(1));

  m_statusbar->addWidget(m_status, 20);
  m_statusbar->addPermanentWidget(label1, 0);
  m_statusbar->addPermanentWidget(m_time, 0);

  this->setCentralWidget(centralWidget);
  this->setStatusBar(m_statusbar);

  QMetaObject::connectSlotsByName(this);
}



void mainWnd::setupActions()
{
  m_fileExit = new QAction("e&Xit", this);
  m_fileExit->setShortcuts(QKeySequence::Quit);
  m_fileExit->setStatusTip("exits the application");
  connect(m_fileExit, &QAction::triggered, this, &mainWnd::onExit);

  m_viewSystem = new QAction("view system", this);
  //m_viewSystem->setShortcuts()
  m_viewSystem->setStatusTip("view the current system configuration");
  connect(m_viewSystem, &QAction::triggered, this, &mainWnd::onViewSystem);

  m_viewCenter = new QAction("center", this);
  //m_fileExit->setShortcuts(QKeySequence::Quit);
  m_viewCenter->setStatusTip("centers the display on the primary");
  connect(m_viewCenter, &QAction::triggered, this, &mainWnd::onViewCenter);

  m_zoomIn = new QAction("zoom in", this);
  m_zoomIn->setShortcuts(QKeySequence::ZoomIn);
  m_zoomIn->setStatusTip("zooms in");
  connect(m_zoomIn, &QAction::triggered, this, &mainWnd::onViewZoomIn);

  m_zoomOut = new QAction("zoom out", this);
  m_zoomOut->setShortcuts(QKeySequence::ZoomOut);
  m_zoomOut->setStatusTip("zooms out");
  connect(m_zoomOut, &QAction::triggered, this, &mainWnd::onViewZoomOut);

  m_zoomFit = new QAction("zoom fit", this);
  //m_fileExit->setShortcuts(QKeySequence::Quit);
  m_zoomFit->setStatusTip("fits the display to the window");
  connect(m_zoomFit, &QAction::triggered, this, &mainWnd::onViewZoomFit);

  m_simConfig = new QAction("configurtion", this);
  m_simConfig->setStatusTip("edit parametes used for simulation");
  connect(m_simConfig, &QAction::triggered, this, &mainWnd::onSimConfig);

  m_simBuild = new QAction("load simulation");
  m_simBuild->setStatusTip("load the simulation from external data");
  connect(m_simBuild, SIGNAL(triggered()), this, SLOT(onReadSystem()));

  m_simGenerate = new QAction("gen simulation");
  m_simGenerate->setStatusTip("generate the simulation");
  connect(m_simGenerate, &QAction::triggered, this, &mainWnd::onGenSystem);

  m_simRun = new QAction("run simulation");
  m_simRun->setStatusTip("run the simulation");
  connect(m_simRun, &QAction::triggered, this, &mainWnd::onRunSimulation);

  m_simPause = new QAction("pause simulation");
  m_simPause->setStatusTip("pause the simulation");
  m_simPause->setEnabled(false);
  connect(m_simPause, &QAction::triggered, this, &mainWnd::onPauseSimulation);

  m_simResume = new QAction("resume simulation");
  m_simResume->setStatusTip("resume the simulation");
  m_simResume->setEnabled(false);
  connect(m_simResume, &QAction::triggered, this, &mainWnd::onResumeSimulation);

  m_simStop = new QAction("stop simulation");
  m_simStop->setStatusTip("stop the simulation");
  connect(m_simStop, &QAction::triggered, this, &mainWnd::onStopSimulation);
}

void mainWnd::setupMenus()
{
  QMenu* fileMenu = menuBar()->addMenu("&file");
  fileMenu->addAction(m_fileExit);

  QMenu* viewMenu = menuBar()->addMenu("&view");
  viewMenu->addAction(m_viewSystem);
  viewMenu->addSeparator();
  viewMenu->addAction(m_viewCenter);
  viewMenu->addAction(m_zoomIn);
  viewMenu->addAction(m_zoomOut);
  viewMenu->addAction(m_zoomFit);

  QMenu* simMenu = menuBar()->addMenu("simulation");
  simMenu->addAction(m_simConfig);
  QMenu* simData = simMenu->addMenu("simulation data");
  simData->addAction(m_simBuild);
  simData->addAction(m_simGenerate);
  QMenu* simCtrls = simMenu->addMenu("simulation controls");
  simCtrls->addAction(m_simRun);
  simCtrls->addAction(m_simPause);
  simCtrls->addAction(m_simResume);
  simCtrls->addAction(m_simStop);
}


void mainWnd::onExit()
{
  QApplication::quit();
}


// configures the properties of the simulation
void mainWnd::onSimConfig()
{
  uint32_t res = QDialog::Rejected;

  simPropertiesDlg       dlg(&m_context);

  res = dlg.exec();
  if (QDialog::Accepted == res)
  {
    dlg.getParams(&m_context);

    if (dlg.didFileNameChange())              // do we have a new config file to use
    {
      onReadSystem(&m_context.datafile);
    }
  }
}


/**********************************************************************************************************************
 * Function: clearOldData
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber) 
 *********************************************************************************************************************/
void mainWnd::clearOldData()
{
  if (!m_dataFile.empty())                 // if m_dataFile is not empty assume second time through this function
  {
    m_dataFile = "";

    if (nullptr != m_system)              // if there is a system structure lurking around, nuke it ...from orbit?
    {
      cleanUpSystem();
      delete m_system;
      m_system = nullptr;
    }
  }
}

void mainWnd::onViewSystem()
{
  if (m_orbitalProperties.size() == 0)
  {
    QMessageBox::information(this, "no system", "A system has not be loaded or generated", QMessageBox::Ok);
  }
  else
  {
    detailsDlg* dlg = new detailsDlg(m_orbitalProperties, m_system, this);
    dlg->exec();

  }

}

// generate a system from a data file
void mainWnd::onReadSystem(std::string* inFile)
{
  if (!m_cmdLineName.empty())                 //  case 1 : if m_cmdLineName is set use it
  {
    clearOldData();                           // remove any existing data
    m_dataFile = m_cmdLineName;
    m_cmdLineName.clear();
  }
  else if (nullptr != inFile)                 //  case 2 : if handed something use it
  {
    clearOldData();
    m_dataFile = *inFile;
  }
  else                                        //  case 3 : ask the user for the file
  {
    QString name = QFileDialog::getOpenFileName(this, "Select data file to use", QDir::currentPath(), "Json files (*.json);;text file (*.txt *.dat)");
    if (!name.isEmpty())
    {
      clearOldData();
      m_dataFile = name.toStdString();
    }
    else                                      // user cancelled the file select dialog
    {
      return;                                 // no name, bail out 
    }
  }

  // one way or another we finaly have a name to use
  if (!m_dataFile.empty())
  {
    m_statusbar->showMessage(QString("using data file %1").arg(m_dataFile.c_str()), 500);
    this->setWindowTitle(QString("Orbital Simulator - data file %1").arg(m_dataFile.c_str()));

    m_context.datafile = m_dataFile;
    m_context.method = system::method::MANUAL;
    QFile inFile(m_dataFile.c_str());                  // have a file name, parse the file 

    if (!inFile.open(QFile::ReadOnly | QFile::Text))
    {
      CLogger::getInstance()->outMsg(cmdLine, CLogger::level::ERR, "failed to open file %s", m_dataFile.c_str());
    }
    else
    {
      QJsonParseError   err;
      QTextStream in(&inFile);
      QString jsonContents = in.readAll();
      inFile.close();

      QByteArray jsonBytes = jsonContents.toLocal8Bit();

      auto jsonDoc = QJsonDocument::fromJson(jsonBytes, &err);

      if (!jsonDoc.isNull())
      {
        if (jsonDoc.isObject())
        {
          m_system = new systemT{ 15 };
          QJsonObject jsonObj = jsonDoc.object();

          if (!jsonObj.isEmpty())
          {
            QVariantMap result = jsonObj.toVariantMap();

            m_system->cntObjects = result["cntObjects"].toInt();

            m_system->timeStep = 0;

            // get the primary properties.
            m_system->primary.mass = result["primary"].toMap()["mass"].toDouble();
            m_system->primary.isl = result["primary"].toMap()["isl"].toDouble();
            m_system->primary.osl = result["primary"].toMap()["osl"].toDouble();


            // determine where the center point of the display (in pixels is locate)
            float maxX = m_system->primary.osl * SCALE;
            float maxY = m_system->primary.osl * SCALE;

            //vector<double_t, 3> center; center.coord(maxX, 0); center.coord(maxY, 1);
            m_center = QPoint(maxX, maxY);
            //vector<double_t, 3> pos = o->curPos * (1 / AU) * SCALE + center;                   // pos vector is in km... need to convert to AU then scale



            // add the primary to list of objects...
            system::objectT* temp1 = new system::objectT{ 0, m_system->primary.mass * MSOL, "primary" };       // mass in kg, and distances in km

            temp1->curPos.coord(0.0l, 0);                // set components of the position vector (along the x axis)
            temp1->curPos.coord(0.0l, 1);
            temp1->curPos.coord(0.0l, 2);

            temp1->curVel.coord(0.0l, 0);                // set components of the velocity vector (parallel to y-axis)
            temp1->curVel.coord(0.0l, 1);
            temp1->curVel.coord(0.0l, 2);

            m_system->objects.push_back(temp1);

            QVariantList planets = result["planets"].toList();
            for (uint32_t ndx = 1; ndx < m_system->cntObjects; ndx++)
            {
              QMap<QString, QVariant> temp = planets[ndx - 1].toMap();

              std::string n = temp["name"].toString().toStdString();
              double a = temp["semimajor"].toDouble();
              double e = temp["eccentricity"].toDouble();
              double i = temp["inclination"].toDouble();
              double L = temp["L"].toDouble();
              double w = temp["argOfPeriapsis"].toDouble();
              double W = temp["longAscNode"].toDouble();
              double m = temp["mass"].toDouble();
              double t = temp["period"].toDouble();
              double o = temp["obliquity"].toDouble();

              system::objectT* temp1 = new system::objectT{ ndx, m * MEARTH, n };       // mass in kg, distances in km

              double sm = a;                      // semi-major axis in AU
              double ecc = e;
              double period = t * DAY2SEC;        // get period in seconds

              double_t circum = 4 * sm * AU2KM* solve2ellipticIntegral(ecc * ecc);      // circumference in km
              double_t speed = circum / period;                                         // speed in km/second

              orbitalPropT    tempProps{ a,e,i,L,w,W,m,t,o,speed };
              strncpy(tempProps.n, n.c_str(), 9);
              m_orbitalProperties.push_back(tempProps);

              // assume a heliocentric coordinate system with x-axis parallel to vector to periapsis, z-axis parallel to angular momentum
              if(ecc < 0.4)
                temp1->curPos.coord(sm*(1-ecc), 0);         // assume that planet is at periapsis (along the x axis)
              else
                temp1->curPos.coord(sm, 0);     // high eccentricity orbits start at apiapsis
              temp1->curPos.coord(0.0l, 1);
              temp1->curPos.coord(0.0l, 2);

              temp1->curVel.coord(0.0l, 0);              // set components of the velocity vector (parallel to y-axis)
              temp1->curVel.coord(speed, 1);
              temp1->curVel.coord(0.0l, 2);

              /*
               TODO : rotate both position and velocity vector by argument of periapsis about z-axis
               TODO : rotate both position and velocity vector by inclination about y-axis
               TODO : rotate both position and velocity vector by RAAN about z-axis
              
                  | X |   |cos W -sin W  0 | | cos I  0  sin I | | cos w -sin w  0 || x |
                  | Y | = |sin W  cos W  0 | |  0     1   0    | | sin w  cos w  0 || y |
                  | Z |   | 0      0     1 | |-sin I  0  cos I | |  0      0     1 || z |
              */

              m_system->objects.push_back(temp1);
            }
          }
          else
          {
            CLogger::getInstance()->outMsg(cmdLine, CLogger::level::WARNING, "JSON object is empty");
          }
        }
        else
        {
          CLogger::getInstance()->outMsg(cmdLine, CLogger::level::WARNING, "JSON does not represent an object");
        }
      }
      else
      {
        CLogger::getInstance()->outMsg(cmdLine, CLogger::level::WARNING, "failed to create JSON document");
      }
    }

    //// print out the orbital parameters.....
    //std::cout << std::setw(12) << "name\t" << "semi-major\t" << "eccent\t" << "inclination\t" << "mean long\t" << "long perapsis\t"
    //  << "long rising\n" << "period\t" << "obliquity" << std::endl;;
    //for (orbitalPropT op : m_orbitalProperties)
    //{
    //  std::cout << op;
    //}
    //// print out the system structure.....
    //std::cout << *m_system << std::endl;
  }
}


// generate initial conditions for accrete to use
void mainWnd::onGenSystem()
{
  // TODO : generate conditions for accretion disk (gas/dust amounts and distribution)
  // TODO : generate random number of nucleii and set period to keplarian period, orbits can be highly eccentric and unstable
  // TODO : configure struct star as appropriate
  // TODO : generate system structure with initial nuclei, set time step to zero.
}


void mainWnd::onViewCenter() 
{
  m_graphicsview->centerOn(m_center);
}


void mainWnd::onViewZoomIn() 
{
  float_t factor = 2.0f;
  m_graphicsview->scale(factor, factor);
}


void mainWnd::onViewZoomOut() 
{
  float_t factor = 0.5f;
  m_graphicsview->scale(factor, factor);
}


void mainWnd::onViewZoomFit() 
{
  m_graphicsview->fitInView(QRect(-m_center.x(), -m_center.y(), 2 * m_center.x(), 2 * m_center.y()), Qt::KeepAspectRatio);
}

/*
see https://www.youtube.com/watch?v=nCg3aXn5F3M for info on this method
*/
/*
  states: stopped - simulation is not running
          running - simulation is running
          paused - simulation is running, but paused
          finished - simulation has finished
          configure - simulation parameters are being configured

  transition funciton:

               | start   | stop     | pause  | restart  | resume  | reset     |
     ----------+---------+----------+--------+----------+---------+-----------+
     stopped   | running | finished |   X    |   X      |   X     | configure |
     running   |   X     |    X     | paused | running* |   X     |    X      |
     paused    |   X     | finished |   X    |   X      | running | configure |
     finished  |   X     |    X     |   X    |   X      |   X     |    X      |
     configure |   X     |    X     |   X    | running  |   X     | running   |

     * transitions to stopped state and then transitions to running state
        
*/
void mainWnd::onRunSimulation()
{
  
  if (m_dataFile.empty())
  {
    m_context.method = system::method::ACCRETE;
    onGenSystem();
  }
    
  m_graphicsview->setScene(m_graphicsScene);

  updateDisplay();                                 // update display showing initial conditions
  onViewCenter();                                  // center display on origin

  // account for different units in duration  "seconds", "minutes", "hours", "days", "months(30 day)", "years"
  double_t factor = 1.0;
  std::string units = m_context.durUnits;
  if (units == "seconds") factor = 1.0;
  else if (units == "minutes") factor = 60.0;
  else if (units == "hours") factor = 3600.0;
  else if (units == "days") factor = 86400.0;
  else if (units == "months(30 day)") factor = 30.0 * 86400.0;
  else factor = 365.25 * 86400;
  uint64_t maxSteps = ceil((m_context.duration * factor) / m_context.deltaT);
  
  m_bRunning = true;
  m_simPause->setEnabled(true);
  
  while (m_bRunning)
  {
    while (!m_bPaused)
    {
      m_status->setText("Simulation Running");
      for (int step = 0; step < maxSteps; step++)
      {
        // (1) calcuate net force on each object in the system, calculating the new position and velocity vectors
        for (uint32_t obj = 0; obj < m_system->cntObjects; obj++)
        {
          vector<double_t, 3> netForce; netForce.coord(0.0, 0); netForce.coord(0.0, 1); netForce.coord(0.0, 2);

          // iterate over objects array and calculate force on each object
          for (uint32_t ndx = 0; ndx < m_system->cntObjects; ndx++)
          {
            //ndx - obj
            if (obj == ndx) continue;                                                     // no force between object and itself...

            vector<double_t, 3> disp = m_system->objects.at(ndx)->curPos - m_system->objects.at(obj)->curPos;                          // units are AU

            double_t R = disp.len();
            netForce = netForce + disp * ((G * m_system->objects.at(obj)->mass * m_system->objects.at(ndx)->mass) / (R * R * R));
          }
          netForce = netForce * (1 / (AU * AU));                                                                                        // units are m kg/sec^2
          m_system->objects.at(obj)->netForce = netForce;

          vector<double_t, 3> acc;
          acc = netForce * (1 / (1000 * m_system->objects.at(obj)->mass));                                                              // units are km/sec^2

          // calculate the new velocity of the object v_{i+1} = v_{i} + a*dt
          m_system->objects.at(obj)->newVel = m_system->objects.at(obj)->curVel + acc * (m_context.deltaT);                              // units are km/sec

          // use updated velocity vector to calcualte new position vector. x_{i+1} = x_i + v_{i+1}*t 
          m_system->objects.at(obj)->newPos = m_system->objects.at(obj)->curPos + m_system->objects.at(obj)->newVel * (1000 * m_context.deltaT / AU);
        } // end object loop

        // tabular display for debugging...
        //printf("%4d;", step);
        //for (uint32_t ndx = 1; ndx < m_system->cntObjects; ndx++)         // iterate over objects, skipping primary
        //{
        //  printf("%.4E, %.4E;\t", m_system->objects.at(ndx)->curPos.coord(0), m_system->objects.at(ndx)->curPos.coord(1));
        //  printf("%.4E, %.4E;\t", m_system->objects.at(ndx)->curVel.coord(0), m_system->objects.at(ndx)->curVel.coord(1));
        //  printf("%.4E, %.4E;\t", m_system->objects.at(ndx)->netForce.coord(0), m_system->objects.at(ndx)->netForce.coord(1));
        //  //printf("[%.4E, %.4E]\t", m_system->objects.at(ndx)->newPos.coord(0), m_system->objects.at(ndx)->newPos.coord(0));
        //  //printf("[%.4E, %.4E]\t", m_system->objects.at(ndx)->newVel.coord(0), m_system->objects.at(ndx)->newVel.coord(0));
        //}
        //printf("\n");
        // ... end tabular display

        // (2) for all objects, update current vectors to the new ones.
        for (uint32_t obj = 0; obj < m_system->cntObjects; obj++)
        {
          m_system->objects.at(obj)->curPos = m_system->objects.at(obj)->newPos;
          m_system->objects.at(obj)->curVel = m_system->objects.at(obj)->newVel;
        }

        // (3) update scene with new positions
        updateDisplay();

        // (3) message pump run
        updateStatusBar(step);
        QApplication::processEvents(QEventLoop::AllEvents);    // let any pending event run
      }
    } // closes while(!m_bPaused) loop....
    // we get here only if the simulation is paused
    QApplication::processEvents(QEventLoop::AllEvents);    // let any pending event run
  } // closes while(m_bRunning) loop....


  m_status->setText("Simulation Finished");
}

void mainWnd::onPauseSimulation()
{
  m_bPaused = true;
  m_status->setText("simulation is paused");
  m_simPause->setEnabled(false);
  m_simResume->setEnabled(true);
}

void mainWnd::onResumeSimulation()
{
  m_bPaused = false;
  m_status->setText("simulation running");
  m_simPause->setEnabled(true);
  m_simResume->setEnabled(false);
}

void mainWnd::onStopSimulation()
{
  m_status->setText("Simulation done");
  onViewCenter();

  m_bRunning = false;
  m_bPaused = false;
}

void mainWnd::updateStatusBar(uint64_t step)
{
  double currTime = m_context.start + ((step * m_context.deltaT / 86400) / 365.25);

  if (m_time != nullptr)
  {
    QString msg = QString("%1 years").arg(currTime);
    m_time->setText(msg);
  }

  // only update the display every tenth iteration step -- ?? is this too frequently
  //if(step % 10 == 0)
  //  updateDisplay();
}


void mainWnd::updateDisplay()
{    
  float maxX = m_system->primary.osl * SCALE;
  float maxY = m_system->primary.osl * SCALE;

  // define colors for upto nine planets and primary
  //                  primary                 grey;                   pale yellow/brown;      blue;                   red/orange;             bands of yellow; 
  //QColor colors[] = { QColor(0xFF,0xFF,0x00), QColor(0x60,0x60,0x60), QColor(0xCC,0xCC,0x00), QColor(0x00,0x00,0xCC), QColor(0xFF,0xB2,0x66), QColor(0x60,0x60,0x60),
    //                white;                  red;                    goldem/pale yellow;     cyan/blue;              deep blue
   //                   QColor(0xFF,0xFF,0xFF), QColor(0xFF,0x00,0x00), QColor(0xFF,0xFF,0x99), QColor(0x00,0x80,0xFF), QColor(0x00,0x00,0x99) };


  // define pens and brushes being used
  QPen  gridPen = QPen(Qt::black);
  gridPen.setCosmetic(true);
  QBrush brush = QBrush(Qt::darkGray);
  QPen   pen = QPen(Qt::SolidLine);

  static bool first = true;
  
  if (first)
  {
    if (m_graphicsScene != nullptr)
    {
      m_graphicsScene->clear();                                     // clear the scene

      // draw grid -- ?? do we need labels every 10 AU?
      m_graphicsScene->addLine(0, maxY, 2 * maxX, maxY, gridPen);
      for (int ndx = 0; ndx < 2 * maxX; ndx += static_cast<int>(SCALE))
      {
        uint32_t len = (ndx % (10 * static_cast<int>(SCALE)) == 0 ? 40 : 20);
        m_graphicsScene->addLine(ndx, maxY - len, ndx, maxY + len, gridPen);
      }
      m_graphicsScene->addLine(maxX, 0, maxX, 2 * maxY, gridPen);
      for (int ndx = 0; ndx < 2 * maxY; ndx += static_cast<int>(SCALE))
      {
        uint32_t len = (ndx % (10 * static_cast<int>(SCALE)) == 0 ? 40 : 20);
        m_graphicsScene->addLine(maxX - len, ndx, maxX + len, ndx, gridPen);
      }
    }
  }
 
  if(nullptr != m_graphicsScene)
  {
    // to determine the size of the object, calculate the factor = floor(log(m_e)) where m_e is mass of object in earth mass units (range -2 to 5)
    // set the bounding box of object to be 6 +/- factor
    int ndx = 0;
    for (system::objectT* o : m_system->objects)
    {
      double_t factor = floor(log(o->mass / MEARTH) + 6);  
      if (factor < 4) factor = 4;                                                     // clamp factor to be at least 4 (useful for comets)
      vector<double_t, 3> center; center.coord(maxX, 0); center.coord(maxY, 1);       // set center locations in pixels
      vector<double_t, 3> pos = o->curPos * SCALE + center;                           // curPos is in AU,
      
      double_t top = pos.coord(1) - factor;
      double_t left = pos.coord(0) - factor;
      if (ndx < sizeof(colors)) {pen.setColor(colors[ndx]);  brush = QBrush(colors[ndx], Qt::SolidPattern);}
      else { pen.setColor(Qt::black); brush = QBrush(Qt::black, Qt::SolidPattern); }
      m_graphicsScene->addEllipse(left, top, 2 * factor, 2 * factor, pen, brush);
      
      o->orbit.push(pos);

      if (m_context.showOrbits)
      {
        QPolygonF orbit;

        for(vector<double_t,3> pt : o->orbit.data())
        {
          orbit << QPoint(pt.coord(0), pt.coord(1));
          
          //m_graphicsScene->addPolygon(orbit, pen);
          QPainterPath path;
          path.addPolygon(orbit);
          m_graphicsScene->addPath(path, pen);
        }
      }

      ndx++;
    }
      
    QCoreApplication::processEvents();
  }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper functions
/**********************************************************************************************************************
 * Function: solve2ellipticIntegral
 *
 * Abstract:  Finds an approximate answer for the integral:
 *                 E(x) = int_0^{\pi/2} \sqrt(1 - xsin^2\theta)d\theta
 *
 *            A numerical solution is readily found by using Simpson's Rule using the midpoint of the interval and
 *            using 20 intervals.
 *
 *            Test cases for this function are:  (this table from CRC Standard Mathematical Tables and Formulae,
 *            33 Edition and MatLab using the ellipticE function)
 *            k^2          0  0.10     0.20     0.30     0.40     0.50     0.60     0.70     0.80     0.90     1.00
 *            E(k)      pi/2  1.531    1.489    1.445    1.399    1.351    1.298    1.242    1.178    1.105    1
 *            calc'd  1.5708  1.53076  1.48904  1.44536  1.39939  1.35064  1.29843  1.24167  1.17849  1.10477  1.00026
 *
 * Input   : x -- [in] eccentricity of the ellipse
 *
 * Returns : float, the value of the complete elliptical integral of the second kind.
 *
 * Written : Aug 2025 (gkhuber)
 *********************************************************************************************************************/
double_t mainWnd::solve2ellipticIntegral(double_t x)
{
  std::function<uint64_t(uint32_t)> fact = [&](uint32_t n)->uint64_t {return n < 2 ? 1 : n * fact(n - 1); };
  std::function<uint64_t(int32_t)> fact2 = [&](int32_t n)->uint64_t {return n < 2 ? 1 : n * fact2(n - 2); };

  double_t deltaA = (PI / 2) / 20;
  double_t sum = 0;
  double_t init = 0.0;
  for(init = 0; fabs(init - (PI/2)) > FLT_EPSILON; init += deltaA)
  {
    double_t angle = init + deltaA / 2;
    double_t sinA = sin(angle);
    sum += sqrt(1.0l - x * sinA * sinA) * deltaA;  }

  //std::cout << "eccentricity " << x << " 2nd elliptic integral is " << sum << std::endl;

  return sum;
}

// clears data about the system
void mainWnd::cleanUpSystem()
{
  if (m_system != nullptr)
  {
    for (auto o : m_system->objects)
    {
      delete o;
      o = nullptr;
    }

    m_system->objects.clear();
    delete m_system;
    m_system = nullptr;
  }

  m_orbitalProperties.clear();
}