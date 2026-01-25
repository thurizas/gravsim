#include "mainWnd.h"
#include "common.h"
#include "constants.h"

#include "2DGraphicsView.h"
#include "simWorker.h"
#include "simPropertiesDlg.h"
//#include "simThread.h"
#include "detailsDlg.h"
#include "utility.h"

#include "libprocgen\procgen.h"

#include <iostream>
#include <iomanip>
#include <random>

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
#include <QJsonArray>
#include <QLabel>
#include <QThread>
#include <QMessageBox>
#include <QPainterPath>

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Aug 2025 (gkhuber)
 **********************************************************************************************************************/
mainWnd::mainWnd(pctxT context, QWidget* p) : QMainWindow(p), m_context(*context)
{
  setupUI();
  setupActions();
  setupMenus();

  m_graphicsScene = new QGraphicsScene;

  m_context.systemMutex = new QMutex;
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Aug 2025 (gkhuber)
 **********************************************************************************************************************/
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


/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Aug 2025 (gkhuber)
 **********************************************************************************************************************/
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


/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Aug 2025 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::setupActions()
{
  m_fileExit = new QAction("e&Xit", this);
  m_fileExit->setShortcuts(QKeySequence::Quit);
  m_fileExit->setStatusTip("exits the application");
  connect(m_fileExit, &QAction::triggered, this, &mainWnd::onExit);

  m_fileExportPrimary = new QAction("save primary", this);
  //m_fileExportPrimary->setShortcuts();
  m_fileExportPrimary->setStatusTip("export the primary star infomation");
  m_fileExportPrimary->setEnabled(false);
  connect(m_fileExportPrimary, &QAction::triggered, this, &mainWnd::onExportPrimary);

  m_fileExportSystem = new QAction("save system", this);
  //m_fileExportSystem->setShortcuts(QKeySequence();
  m_fileExportSystem->setStatusTip("export the system desctiption in JSON or YAML");
  m_fileExportSystem->setEnabled(false);
  connect(m_fileExportSystem, &QAction::triggered, this, &mainWnd::onExportSystem);

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

  m_simConfig = new QAction("configuration", this);
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


/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Aug 2025 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::setupMenus()
{
  QMenu* fileMenu = menuBar()->addMenu("&file");
  fileMenu->addAction(m_fileExportPrimary);
  fileMenu->addAction(m_fileExportSystem);
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


/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Aug 2025 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onExit()
{
  QApplication::quit();
}


/***********************************************************************************************************************
 * Function: 
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber) 
 **********************************************************************************************************************/
void mainWnd::onExportPrimary()
{
  QString outName = "";

  if (nullptr != m_system)
  {
    outName = QFileDialog::getSaveFileName(nullptr, "save primary data", QDir::currentPath(), "Json files (*.json);;Text files (*.txt *.dat)");

    if (outName != "")
    {
      m_statusbar->showMessage(QString("saving primary data it %1").arg(outName.toStdString().c_str()), 500);


      QFile outFile(outName);                  // have a file name, parse the file 

      if (!outFile.open(QFile::ReadWrite | QFile::Truncate | QFile::Text))
      {
        CLogger::getInstance()->outMsg(cmdLine, CLogger::level::ERR, "failed to open file %s", m_dataFile.c_str());
      }
      else
      {
        QJsonParseError   err;

        QJsonObject  rootObject;
        QJsonObject  primaryObject;
        primaryObject.insert("earthlike", m_system->primary.earthLike);
        primaryObject.insert("mass", m_system->primary.mass);
        primaryObject.insert("age", m_system->primary.age);
        primaryObject.insert("max-age", m_system->primary.maxage);
        primaryObject.insert("radius", m_system->primary.radius);
        primaryObject.insert("lumin", m_system->primary.lumins);
        primaryObject.insert("temp", m_system->primary.temp);
        primaryObject.insert("isl", m_system->primary.isl);
        primaryObject.insert("osl", m_system->primary.osl);

        rootObject.insert("primary", primaryObject);

        QJsonDocument jsonDoc(rootObject);

        
        QByteArray outData = jsonDoc.toJson();
        outFile.write(outData);

        outFile.close();
      }
    }
    else
    {
      QMessageBox::information(nullptr, "Information", "operation canceled by user");
    }
  }
  else
  {
    QMessageBox::warning(nullptr, "Missing Data", "No data to export found");
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::WARNING, "failed to export primary data - no data found");
  }
}
/**********************************************************************************************************************
* Function: 
* 
* Abstract:
* 
* Input   :
* 
* Returns :
* 
* Written : Jan 2026 (gkhuber)  
**********************************************************************************************************************/
void mainWnd::onExportSystem()
{

}

/**********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Aug 2025 (gkhuber)
 **********************************************************************************************************************/
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

    updateDisplay(0);
    onViewCenter();
  }
}


/***********************************************************************************************************************
 * Function: clearOldData
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber) 
 **********************************************************************************************************************/
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

  m_fileExportPrimary->setEnabled(false);
  m_fileExportSystem->setEnabled(false);
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Aug 2025 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onViewSystem()
{
  if (m_system == nullptr)
  {
    QMessageBox::information(this, "no system", "A system has not be loaded or generated", QMessageBox::Ok);
  }
  else
  {
    detailsDlg* dlg = new detailsDlg(m_system, this);
    dlg->exec();

  }

}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Dec 2025 (gkhuber)
 **********************************************************************************************************************/
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

    // prepare a new system structure
    if (nullptr != m_system)
    {
      clearOldData();
    }
    m_system = new systemT{ system::method::MANUAL };

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

              temp1->orbProps = orbitalPropT{ a,e,i,L,w,W,t,o,speed };

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


/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::getStellarFile()
{
  QString inFile = QFileDialog::getOpenFileName(this, "Select primary data file", QDir::currentPath(), "Json files (*.json);;Text files (*.txt *.dat)");
  if (inFile != "")
    m_context.stellarDataFile = inFile.toStdString();
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber)
 **********************************************************************************************************************/
bool mainWnd::readStellarFile()
{
  bool res = false;
  m_statusbar->showMessage(QString("using primary data file %1").arg(m_context.stellarDataFile.c_str()), 500);
  this->setWindowTitle(QString("Orbital Simulator - primary name %1").arg(m_context.stellarDataFile.c_str()));

  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "reading primary file %s\n", m_context.stellarDataFile.c_str());

  QFile inFile(m_context.stellarDataFile.c_str());                  // have a file name, parse the file 

  if (!inFile.open(QFile::ReadOnly | QFile::Text))
  {
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::ERR, "failed to open file %s", m_context.stellarDataFile.c_str());
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

          // get the primary properties.
          m_system->primary.earthLike = result["primary"].toMap()["earthLike"].toBool();
          m_system->primary.mass = result["primary"].toMap()["mass"].toDouble();
          m_system->primary.age = result["primary"].toMap()["age"].toDouble();
          m_system->primary.maxage = result["primary-age"].toMap()["max-age"].toDouble();
          m_system->primary.radius = result["primary"].toMap()["radius"].toDouble();
          m_system->primary.lumins = result["primary"].toMap()["lumin"].toDouble(); 
          m_system->primary.temp = result["primary"].toMap()["temp"].toDouble();
          m_system->primary.isl = result["primary"].toMap()["isl"].toDouble();
          m_system->primary.osl = result["primary"].toMap()["osl"].toDouble();

          res = true;
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

  return res;
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 * 
 * step 1 -- determine star classification. table of types (main sequence stars from https://www.youtube.com/watch?v=hG1of0MroM8
 * 
 *   class | frequence  |  mass        |range         |  color         | notes
 *     O   |  0.00003   |  \ge 16      |    1 -    1  | 155, 176, 255  | slight underestimate of type O stars
 *     B   |  0.13000   |  2.1 - 16    |    2 -   15  | 170, 191, 255  |
 *     A   |  0.60000   |  1.40 - 2.10 |  16 -   76   | 202, 215, 255  |
 *     F   |  3.00000   |  1.04 - 1.40 |  77 -  377   | 248, 247, 255  |
 *     G   |  7.60000   |  0.80 - 1.04 |  378 - 1138  | 255, 244, 234  |
 *     K   | 12.10000   |  0.45 - 0.80 | 1139 - 2349  | 255, 210, 161  |
 *     M   | 76.45000   |  0.08 - 0.45 | 2350 - 9995  | 255, 204, 111  |
 * 
 * step 2 -- determine the star mass from above table, use a linear approximation to select from range
 * 
 * step 3 - generate max time on main-sequence, \tau_{ms} = 10^{10}(\frac{M}{M_sol})^{-2.5}
 *
 * step 3 -- generate radius, R = M^{0.5} is M \ge M_{sol} else R = M^{0.8} if M < M_{sol}
 *
 * step 4 - generate luminosity, 
 *                  = 0.23(\frac{M}{M_sol})^{2.3}L_sol             if M < 0.43M_sol
 *                  = (\frac{M}{M_sol})^4L_sol                     if 0.43M_sol < M < 2M_sol
 *                L = 1.4(\frac{M}{M_sol})^{3.5}L_sol              if 2M_sol < M <55M_sol
 *                  = 32000\frac{M}{M_sol}L_sol
 * 
 *               L = 4\pi\sigma\R^2T^4 = (\frac{M}{M_sol})^a * L_sol  (stephen-boltzmann formula)
 *
 * step 5 - calculate the effective temperature T = (L/R^2)^{0.25}
 *  
 * some represenative values:
 * 
 * stellar Mass, Msol      0.8 	     1.1        7.0        25.0          80.0
 * max  age, Gyr          19.531     7.512      0.055       0.002         0.000
 * radius, Rsol            0.837     1.056      3.032       6.264        12.155
 * luminosity, Lsol        0.410     1.464   1270.490  109375.00    6411254.000
 * density, Dsol           1.367     0.935      0.251       0.102         0.045
 * temperature, K       5052.208  6183.337  19804.645   41970.469     83364.719
 * isl, AU                 0.007     0.953      0.014       0.021         0.031    
 * osl, AU                31.607    39.507    276.413     987.163      3158.769
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::genPrimaryStar(bool sunlike)
{
  typedef struct data
  {
    double_t   up_limit;
    double_t   mass_min;
    double_t   mass_max;
    char       type;
  }  dataT;

  dataT val[7] = { {1,16,99,'O'}, {15,2.10,16,'B'}, {76,1.40,2.10,'A'},{377,1.04,1.40,'F'},
                   {1138,0.80,1.04,'G'}, {2349,0.45,0.80,'K'}, {9995,0.08,0.80,'M'} };

  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "generating a new primary star\n");
  //  step 1 - determine mass
  std::random_device              rd;
  std::mt19937                    gen(rd());
  std::uniform_int_distribution<> dist(1, 9995);
  std::uniform_int_distribution<> dist1(1, 100);
  
  uint32_t ndx = 0;
  if (sunlike)                                              // pick G (38%) or K (62%) to keep relative proportions constant
  {
    uint32_t roll = dist1(gen);

    if (roll <= 38) ndx = 4;
    else ndx = 5;
  }
  else
  {
    uint32_t roll = dist(gen);

    for (; ndx < sizeof(val) / sizeof(val[0]); ndx++)
    {
      if (roll <= val[ndx].up_limit)
      {
        break;
      }
    }
  }
  m_system->primary.mass = val[ndx].mass_min + (val[ndx].mass_max - val[ndx].mass_min) * (static_cast<double_t>(dist1(gen)) / 100.0);

  // step 2 - generate radius
  m_system->primary.radius = calcRadius(m_system->primary.mass);

  // step 3 - generate luminosity - relative to sol
  m_system->primary.lumins = calcLum(m_system->primary.mass);

  //step 4 - calculate the effective temperature T = (L / R ^ 2)^ { 0.25 }
  m_system->primary.temp = calcTemp(m_system->primary.mass);

  //step 5 - calculate the max age on the main sequence
  m_system->primary.maxage = calcMAge(m_system->primary.mass)*1000;

  //step 6 - calculate the system limits
  m_system->primary.isl = calcISL(m_system->primary.mass);
  m_system->primary.osl = calcOSL(m_system->primary.mass)/AU;

  //step 7 - determine current age of the star
  m_system->primary.age = (static_cast<double_t>(dist1(gen)) / 100.0) * m_system->primary.maxage;

  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "   mass of primary %.4f solar masses", m_system->primary.mass);
  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "   radius of primary %.4f solar radius", m_system->primary.radius);
  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "   luminosity of primary %.4f solar luminosity", m_system->primary.lumins);
  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "   surface temperature %.4f", m_system->primary.temp);
  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "   max age of primary %.4f GYr", m_system->primary.maxage / 1000);
  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "   current age of primary %.4f GYr", m_system->primary.age / 1000);
  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "   system limits [%.4f, %.4f]", m_system->primary.isl, m_system->primary.osl);

  m_fileExportPrimary->setEnabled(true);
 }


/**********************************************************************************************************************
 * Function: 
 *
 * Abstract: mass of the disk is 0.001 to 0.1 M_star, common estimate is 0.01 M_star. sample from normal distribution 
             \mu = 0.01 \sigma = 1.0
             divide disk into n - segments

             \rho_1 = A\exp{-\alpha r^{1/n}}                              (Dole, 1969)

             A = 1.5E-3 solar masses/a.u.^3
             \alpha = 5
             n = 3
             K = 50

             for exocone h(r) = x\tan(\theta)

             calculation of volumn of exocone:
             V_{shell} = 2*f(x_i)(pi x_i^2 - pi x_{x-i}^2) = 2*pi f(x_i)((x_i - x_{i-1})(x_i + x_{i-1})
                       = 2 pi f(x_i) \frac{x_i + x_{i-1}}{2} (x_i - x_{i-1})
                       =  pi f(x_i^*)x_i^* dx                                 x^* represent height at midpoint

            V_{exocone} = 2\int_{r_0}^{r_1} ( pi x f(x)) dx                // 2 - accounts for top and bottom triangles
                        = 2\int_{r_0}^{r_1} pi x x\tan(\theta) dx
                        = 2 \tan(\theta) pi \int_{r_0}^{r_1} x^2 dx        
                        = \frac{2\tan(\theta) pi}{3} [x^3|_{r_0}^{r_1} =\frac{4\tan(\theta) pi}{3}(r_1^3 - r_0^3)
 *  
 
 // TODO : generate conditions for accretion disk (gas/dust amounts and distribution)
  //        let a = semi-major axis (in AU), \Sigma(a) = surface density at a \Sigma_0 = normalization constant( ~4200 g/cm^3 (Weidenschilling), ~1700 g/cm^3 (Hayashi), or ~50500 g/cm^3 (Desch)
  //            \beta ~= 1.5
  //        \Sigma(a) = \Sigma_0(a/ 1 A)^{-\beta}
  //      : ~60 M_earth of solids ~0.02 M_sol gasses (75% H_2, 25% He)
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber) 
 *********************************************************************************************************************/
void mainWnd::genProtoplanetaryDisk()
{
  std::random_device   rd;
  std::mt19937         mt(rd());

  std::uniform_int_distribution<> intDist(1,100);                // distributions for percentages


  double_t    volumnDisk;
  double_t    dustMass;
  double_t    gasMass;
  double_t    dispAngle = m_context.accreteCtx.dispAngle * DEG2RAD;             

  // (1) calculate volume of exocone, using shell method.
  volumnDisk = ((2.0 * PI * tan(dispAngle))/3) * (pow(m_system->primary.osl, 3) - pow(m_system->primary.isl, 3));

  // (2) calculate mass of dust in exocone sample from normal distribution with \mu = 0.01
  // 0.001 to 0.1 M_star, common estimate is 0.01 M_star
  double_t mu = 0.01;
  double_t sigma = 0.05;
  std::normal_distribution<double_t> ln(mu, sigma);

  double_t val = ln(mt);
  if (val < 0.001) val = 0.001;
  if (val > 0.1) val = 0.1;
  
  dustMass = m_system->primary.mass * val;
  gasMass = m_context.accreteCtx.gasDustRatio * dustMass; 


  // (3) update system structure with known values
  m_system->method = system::method::MANUAL;
  m_system->cntObjects = m_context.accreteCtx.cntNuclei;

  // (3) generate partitions of the cloud and density per partitions.


  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "disk volume : %.4f AU^3", volumnDisk);
  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "dust mass in disk: %.4f solar unit", dustMass);
  CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "gas mass in disk: %.4f solar unit", gasMass);

}


void mainWnd::genPlanetaryNucleii()
{
  std::random_device   rd;
  std::mt19937         mt(rd());

  std::uniform_int_distribution<> intDist(1, 100);                          // distributions for percentages in range [1, 100]
  std::normal_distribution<double_t> normDist(m_context.accreteCtx.initialMass, 0.35 * m_context.accreteCtx.initialMass);
  std::uniform_real_distribution<> realUDist(0, 360);                       // uniform distribution in range [0,360)

  for (uint32_t ndx = 0; ndx < m_context.accreteCtx.cntNuclei; ndx++)
  {
    double_t mass = normDist(mt); 

    char buf[10];
    sprintf(buf, "%d", ndx);
    std::string n = std::string("nucleus") + std::string(buf);

    //instantiate new objectT object
    system::objectT* temp1 = new system::objectT{ ndx, mass, n };       // mass in kg, distances in km
    
    // now calculate the orbital properties
    uint32_t pct = intDist(mt);                                        // generate from a uniform distribution
    temp1->orbProps.a = m_system->primary.isl + (m_system->primary.osl - m_system->primary.isl) * (static_cast<double>(pct) / 100.0);
    temp1->orbProps.e = 1.0 - pow((1.0 - intDist(mt) / 100.0), 0.077);

    temp1->orbProps.i = m_context.accreteCtx.dispAngle * (static_cast<double>(intDist(mt))/100.0);
    temp1->orbProps.w = realUDist(mt);
    temp1->orbProps.W = realUDist(mt);
    temp1->orbProps.t = sqrt((4 * PI * PI * pow(temp1->orbProps.a * AU, 3)) / (G * m_system->primary.mass * MSOL));
    
    double_t circum = 4 * temp1->orbProps.a * AU2KM * solve2ellipticIntegral(temp1->orbProps.e * temp1->orbProps.e);      // circumference in km
    double_t speed = circum / temp1->orbProps.t;                                                                          // speed in km/second


    // assume a heliocentric coordinate system with x-axis parallel to vector to periapsis, z-axis parallel to angular momentum
    if (temp1->orbProps.e < 0.4)
      temp1->curPos.coord(temp1->orbProps.a * (1 - temp1->orbProps.e), 0);         // assume that planet is at periapsis (along the x axis)
    else
      temp1->curPos.coord(temp1->orbProps.a, 0);                                   // high eccentricity orbits start at apiapsis
    temp1->curPos.coord(0.0l, 1);
    temp1->curPos.coord(0.0l, 2);

    temp1->curVel.coord(0.0l, 0);                                                  // set components of the velocity vector (parallel to y-axis)
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
  
  for (uint32_t ndx = 1; ndx < m_context.accreteCtx.cntNuclei; ndx++)
  {
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "    name                       : %s", m_system->objects.at(ndx)->name.c_str());
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "    semi-major, AU             : %.4f", m_system->objects.at(ndx)->orbProps.a);
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "    mass,kg                    : %.4E", m_system->objects.at(ndx)->mass*MSOL);
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "    inclination, deg           : %.2f", m_system->objects.at(ndx)->orbProps.i);
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "    eccentricity               : %.4f", m_system->objects.at(ndx)->orbProps.e);
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "    longitude of ascending node: %.2f", m_system->objects.at(ndx)->orbProps.W);
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::DEBUG, "    argument of periapsis      : %.2f", m_system->objects.at(ndx)->orbProps.w);
  }
}


/***********************************************************************************************************************
 * Function: 
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber) 
 **********************************************************************************************************************/
void mainWnd::onGenSystem()
{  
  if(nullptr != m_system)
  { 
    clearOldData();
  }

  m_system = new systemT{ system::method::ACCRETE };

  // (1) - generate the primary star
  // check if user provided a star data file if so use it
  if (m_context.stellarDataFile == "")
  {
    getStellarFile();
  }

   
  if (m_context.stellarDataFile != "")             // if not star data file, ask for one
  {
    if (!readStellarFile())
    {
      CLogger::getInstance()->outMsg(cmdLine, CLogger::level::ERR, "failed to read primary data file.");
      return;
    }
  }
  else                                             // no data file given, generate a new star
  {
    if (m_context.earthLike == system::earthlike::INDETERMINATE)
    {
      int ret = QMessageBox::question(nullptr, "earth-like star", "generate an earth-like star?", QMessageBox::Yes, QMessageBox::No);
      if (ret == QMessageBox::Yes)
        m_context.earthLike = system::earthlike::YES;
    }

      genPrimaryStar(m_context.earthLike);
  }

  // (2) generate exocone properties (a) mass of disk, (b) density of dust/gas in terms of radii
  //     mass of the disk is 0.001 to 0.1 M_star, common estimate is 0.01 M_star. sample from normal distribution \mu = 0.01 \sigma = 1.0
  genProtoplanetaryDisk();

  // (3) genrate protoplanet nucleii
  genPlanetaryNucleii();


  // 
  // TODO : set deltaT to 100000years
  // TODO : set duration = 3000000years
  // TODO : set maxStep = duration/deltaT
  // TODO : do step = 0 to maxStep
  // TODO :     CoreAccretion (planetessiamls sweep out annulus of ~hill radius)
  // TODO :     GasAccretion   (like core accretion by happens when planets mass > critical mass)
  // TODO :     PebbleAccretion (radially directed flux ~100M_earth/100Myr, small 1mm to 1cm tracked per annulii)
  // TODO :     PlanetaryMigration
  // TODO :     Collisions
  // TODO :     Update
  // TODO : loop
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Dec 2025 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onViewCenter() 
{
  m_graphicsview->centerOn(m_center);
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Dec 2025 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onViewZoomIn() 
{
  float_t factor = 2.0f;
  m_graphicsview->scale(factor, factor);
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Dec 2025 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onViewZoomOut() 
{
  float_t factor = 0.5f;
  m_graphicsview->scale(factor, factor);
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Dec 2025 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onViewZoomFit() 
{
  m_graphicsview->fitInView(QRect(-m_center.x(), -m_center.y(), 2 * m_center.x(), 2 * m_center.y()), Qt::KeepAspectRatio);
}


/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 * 
 * 
 * states: stopped - simulation is not running
 *         running - simulation is running
 *         paused - simulation is running, but paused
 *         finished - simulation has finished
 *         configure - simulation parameters are being configured
 *
 * transition funciton:
 *
 *              | start   | stop     | pause  | restart  | resume  | reset     |
 *    ----------+---------+----------+--------+----------+---------+-----------+
 *    stopped   | running | finished |   X    |   X      |   X     | configure |
 *    running   |   X     |    X     | paused | running* |   X     |    X      |
 *    paused    |   X     | finished |   X    |   X      | running | configure |
 *    finished  |   X     |    X     |   X    |   X      |   X     |    X      |
 *    configure |   X     |    X     |   X    | running  |   X     | running   |
 *
 *    * transitions to stopped state and then transitions to running state
 * 
 * 
 * 
 * 
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onRunSimulation()
{
  if (m_dataFile.empty())
  {
    m_context.method = system::method::ACCRETE;
    onGenSystem();
  }
    
  m_graphicsview->setScene(m_graphicsScene);

  updateDisplay(0);                                // update display showing initial conditions
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

  // start simulation thead here

  simWorker* worker = new simWorker(&m_context, m_system);
  QThread* simWorkerThread = new QThread();
  worker->moveToThread(simWorkerThread);
  connect(simWorkerThread, &QThread::started, worker, &simWorker::start);
  connect(simWorkerThread, &QThread::finished, worker, &simWorker::deleteLater);
  connect(simWorkerThread, &QThread::finished, worker, &QThread::deleteLater);
  connect(worker, &simWorker::updateStatusBar, this, &mainWnd::updateStatusMsg);
  connect(worker, &simWorker::updateGuiThread, this, &mainWnd::updateGuiThread);

  simWorkerThread->start();

}


/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onPauseSimulation()
{
  m_bPaused = true;
  m_status->setText("simulation is paused");
  m_simPause->setEnabled(false);
  m_simResume->setEnabled(true);
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onResumeSimulation()
{
  m_bPaused = false;
  m_status->setText("simulation running");
  m_simPause->setEnabled(true);
  m_simResume->setEnabled(false);
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::onStopSimulation()
{
  m_status->setText("Simulation done");
  onViewCenter();

  m_bRunning = false;
  m_bPaused = false;
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::updateStatusMsg(QString msg)
{
  m_status->setText(msg);
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Jan 2026 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::updateGuiThread(uint64_t step, prenderInfoT pinfos)
{
  double currTime = m_context.start + ((step * m_context.deltaT / 86400) / 365.25);
  std::cerr << "time is: " << step << std::endl;

  if (m_time != nullptr)
  {
    QString msg = QString("%1 years").arg(currTime);
    m_time->setText(msg);
  }

  updateDisplay(pinfos);
}

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Nov 2025 (gkhuber)
 **********************************************************************************************************************/
void mainWnd::updateDisplay(prenderInfoT pinfo)
{    
  // if m_system has not be instantiated yet, just return
  if (nullptr == m_system) return;


  m_context.systemMutex->lock();
  if (nullptr != pinfo)
  {
    std::cerr << "updating display for time: " << m_system->timeStep << "\n";  
    for (uint32_t ndx = 0; ndx < m_system->cntObjects; ndx++)    
    {
      renderInfoT  obj = pinfo[ndx];
      std::cerr << pinfo[ndx] << "\n";
    }
  }


    float maxX = m_system->primary.osl * SCALE;
    float maxY = m_system->primary.osl * SCALE;
  m_context.systemMutex->unlock();

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
    first = false;
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
    m_graphicsScene->update();}
  
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

/***********************************************************************************************************************
 * Function:
 *
 * Abstract:
 *
 * Input   :
 *
 * Returns :
 *
 * Written : Aug 2025 (gkhuber)
 **********************************************************************************************************************/
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
}