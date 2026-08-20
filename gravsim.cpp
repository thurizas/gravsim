#include "libprocgen\procgen.h"
#include "constants.h"
#include "common.h"
#include "mainWnd.h"


#include <QApplication>

#include <iostream>


void showVersion(const char*);
void showUsage(const char*);

int main(int argc, char** argv)
{
  int32_t   nres = -1;
  int32_t   choice = -1;
  int8_t    dbgLevel = CLogger::level::INFO;
  std::string dataFileName = "";
  std::string stellarFileName = "";

  allocConsole();

// default values for the context
  ctxT context{ 0, 24 * 3600, 400, "days", true, system::method::UNKNOWN, system::earthlike::INDETERMINATE, "", "", nullptr};
// default values for the accretion process
  context.accreteCtx = accreteVars{ 20.0, 0.001, 0.01, 0.1, 50.0, 30, 20, 10E-15, 5.0, 3.0, 1.5E-3 };
  
  while (-1 != (choice = getopt(argc, argv, "f:S:dhv")))
  {
    switch (choice)
    {
      case 'f':
        dataFileName = std::string(getOptArg());
        context.datafile = dataFileName;
        context.method = system::method::MANUAL;
        break;

      case 'S':
        stellarFileName = std::string(getOptArg());
        context.stellarDataFile = stellarFileName;
        context.method = system::method::ACCRETE;
        break;

      case 'd':
        dbgLevel--;
        if (dbgLevel <= 0) dbgLevel = CLogger::level::INFO;
        break;

      case 'v':
        showVersion(argv[0]);
        exit(0);

      case '?':
        std::cout << "Unknown command line option " << argv[getInd() - 1] << std::endl;
        [[fallthrough]];
      case 'h':
        showUsage(argv[0]);
        exit(0);
    }
  }

  CLogger* pLogger = CLogger::getInstance();
  pLogger->regOutDevice(cmdLine, cmdColorOut);
  pLogger->outMsg(cmdLine, CLogger::level::SUCCESS, "logging engine successfully instantiated");

  QApplication theApp(argc, argv);

  mainWnd mainwnd(&context);
  if (!dataFileName.empty())
  {
    pLogger->outMsg(cmdLine, CLogger::level::DEBUG, "reading system information from %s", dataFileName.c_str());
    mainwnd.readDataFile(dataFileName);

  }
  mainwnd.show();

  nres = theApp.exec();



  pLogger->delInstance();
  deallocConsole();
  return nres;
}

void showVersion(const char* name)
{
  std::cout << name << " an orbital simulator." << std::endl;
  std::cout << "version: " << static_cast<int>(MAJOR) << "." << static_cast<int>(MINOR) << "." << static_cast<int>(PATCH) << std::endl;
}



void showUsage(const char* name)
{
  std::cout << name << "an orbital simulator." << std::endl;
  std::cout << "usage: " << name << " [options] " << std::endl;
  std::cout << "\noptions: " << std::endl;

  "f:S:dhv";
  std::cout << "f f                   uses the file 'f' for full system data (primary & planet data)" << std::endl;
  std::cout << "S f                   uses the file 'f' for information on the primary" << std::endl;
  std::cout << "d                     increases the verbosity of logging messages, can by use multiple times" << std::endl;
  std::cout << "v                     prints program version and exits" << std::endl;
  std::cout << "h                     displays a synopsis of commands (this screen), and exits" << std::endl;
}