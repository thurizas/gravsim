#include "mainWnd.h"
#include "common.h"
#include "constants.h"

#include "simWorker.h"
#include "simPropertiesDlg.h"

#include "libprocgen\procgen.h"

#include <iostream>
#include <iomanip>

#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QThread>


mainWnd::mainWnd(pctxT context, QWidget* p) : QMainWindow(p), m_context(*context)
{
  setupUI();
  setupActions();
  setupMenus();
}


mainWnd:: ~mainWnd()
{

}


void mainWnd::readDataFile(std::string name)
{
  QFile inFile(name.c_str());

  if (inFile.exists())
  {
    m_dataFile = name;
    onReadSystem();

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

  m_graphicsview = new QGraphicsView();
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

  m_simConfig = new QAction("configurtion", this);
  m_simConfig->setStatusTip("edit parametes used for simulation");
  connect(m_simConfig, &QAction::triggered, this, &mainWnd::onSimConfig);


  // this should have a pop-up menu for read data from a file or generate data
  m_simBuild = new QAction("load simulation");
  m_simBuild->setStatusTip("load the simulation from external data");
  connect(m_simBuild, &QAction::triggered, this, &mainWnd::onReadSystem);

  m_simGenerate = new QAction("gen simulation");
  m_simGenerate->setStatusTip("generate the simulation");
  connect(m_simGenerate, &QAction::triggered, this, &mainWnd::onGenSystem);

  m_simRun = new QAction("run simulation");
  m_simRun->setStatusTip("run the simulation");
  connect(m_simRun, &QAction::triggered, this, &mainWnd::onRunSimulation);
}

void mainWnd::setupMenus()
{
  QMenu* fileMenu = menuBar()->addMenu("&file");
  fileMenu->addAction(m_fileExit);

  QMenu* simMenu = menuBar()->addMenu("simulation");
  simMenu->addAction(m_simConfig);
  QMenu* simData = simMenu->addMenu("simulation data");
  simData->addAction(m_simBuild);
  simData->addAction(m_simGenerate);
  simMenu->addAction(m_simRun);
}


void mainWnd::onExit()
{
  QApplication::quit();
}

// configures the properties of teh simulation
void mainWnd::onSimConfig()
{
  uint32_t res = QDialog::Rejected;

  simPropertiesDlg       dlg(&m_context);

  res = dlg.exec();
  if (QDialog::Accepted == res)
  {
    dlg.getParams(&m_context);
    // TODO : process new simulation parameters
  }

}

// generate a system from a data file
void mainWnd::onReadSystem()
{
  std::string fileName = "";

  if (m_dataFile.empty())                         // data file is empty ask user for a file
  {
    QString name = "";
    name = QFileDialog::getOpenFileName(this, "Select data file to use", QDir::currentPath(), "text file (*.txt *.dat);;Json files (*.json)");
    if (!name.isEmpty())
    {
      fileName = name.toStdString();
    }
    else
    {
      return;
    }
  }
  else
  {
    fileName = m_dataFile;
  }

  m_context.datafile = fileName;
  m_context.method = system::method::MANUAL;
  QFile inFile(fileName.c_str());                  // have a file name, parse the file 

  if (!inFile.open(QFile::ReadOnly | QFile::Text))
  {
    CLogger::getInstance()->outMsg(cmdLine, CLogger::level::ERR, "failed to open file %s", fileName.c_str());
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
        if (nullptr != m_system)              // if there is a system structure lurking around, nuke it ...from orbit?
        {
          cleanUpSystem();
          delete m_system;
        }

        m_system = new systemT{ 15};
        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.isEmpty())
        {
          QVariantMap result = jsonObj.toVariantMap();

          m_system->cntObjects = result["cntObjects"].toInt();

          // get the primary properties.
          m_system->primary.mass = result["primary"].toMap()["mass"].toDouble();
          m_system->primary.isl = result["primary"].toMap()["isl"].toDouble();
          m_system->primary.osl = result["primary"].toMap()["osl"].toDouble();

          // add the primary to list of objects...
          system::objectT* temp1 = new system::objectT{ 0, m_system->primary.mass*MSOL, "primary"};       // mass in kg, and distances in km

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
            QMap<QString, QVariant> temp = planets[ndx-1].toMap();
            
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

            orbitalPropT    tempProps{a,e,i,L,w,W,m,t,o};
            strncpy(tempProps.n, n.c_str(), 9);
            m_orbitalProperties.push_back(tempProps);

            system::objectT* temp1 = new system::objectT{ ndx, m*MEARTH, n };                         // mass in kg, distances in km

            double sm = a*AU;                 // semi-major axis in kilometers
            double ecc = e;
            double period = t*DAY2SEC;        // get period in seconds

            double_t circum = 4 * sm * solve2ellipticIntegral(ecc*ecc);      // circumference in km
            double_t speed = circum / period;                                // speed in km/second

            temp1->curPos.coord(sm, 0);                // set components of the position vector (along the x axis)
            temp1->curPos.coord(0.0l, 1);
            temp1->curPos.coord(0.0l, 2);

            temp1->curVel.coord(0.0l, 0);              // set components of the velocity vector (parallel to y-axis)
            temp1->curVel.coord(speed, 1);
            temp1->curVel.coord(0.0l, 2);

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

    //// print out the orbital parameters.....
    //std::cout << std::setw(12) << "name\t" << "semi-major\t" << "eccent\t" << "inclination\t" << "mean long\t" << "long perapsis\t"
    //  << "long rising\n" << "period\t" << "obliquity" << std::endl;;
    //for (orbitalPropT op : m_orbitalProperties)
    //{
    //  std::cout << op;
    //}
    // print out the system structure.....
    std::cout << *m_system << std::endl;

  }
}

// generate initial conditions for accrete to use
void mainWnd::onGenSystem()
{
  // TODO : generate conditions for accretion disk (gas/dust amounts and distribution)
  // TODO : generate random number of nucleii and set period to keplarian period, orbits can be highly eccentric and unstable
  // TODO : configure struct star as appropriate
}


void mainWnd::onRunSimulation()
{
  if (m_dataFile.empty())
  {
    m_context.method = system::method::ACCRETE;
    onGenSystem();
  }
    

  QThread* m_simWorkerThread = new QThread();
  simWorker* m_simWorker = new simWorker(&m_context, m_system, nullptr);

  m_simWorker->moveToThread(m_simWorkerThread);

  connect(m_simWorkerThread, &QThread::started, m_simWorker, &simWorker::start);
  connect(m_simWorker, &simWorker::tick, this, &mainWnd::updateStatusBar);
  //connect(m_simWorkerThread, &QThread::finished, w, &QThread::deleteLater);
  connect(m_simWorker, &simWorker::finished, this, &mainWnd::onStopSimulation);

  m_simWorkerThread->start();
  m_status->setText("Simulation Running");

}

void mainWnd::onStopSimulation()
{

  delete m_simWorkerThread;
  delete m_simWorker;

}


void mainWnd::updateStatusBar(uint64_t step)
{
  double currTime = m_context.start + ((step * m_context.deltaT / 86400) / 365.25);

  if (m_time != nullptr)
  {
    QString msg = QString("%1 years").arg(currTime);
    m_time->setText(msg);
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

void mainWnd::cleanUpSystem()
{

}