#include "libprocgen\procgen.h"
#include "constants.h"
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

  allocConsole();

  // context: start    : 2000
  //          deltaT   : 24*3600 seconds
  //          duration : 0.00273785078713210130047912388775 years (or ~1 day)
  ctxT context{ 2000, 24 * 3600, 0.0027378508 };
  
  while (-1 != (choice = getopt(argc, argv, "f:dhv")))
  {
    switch (choice)
    {
      case 'f':
        dataFileName = std::string(getOptArg());
        context.datafile = dataFileName;
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
  std::cout << "v                     prints program version and exits" << std::endl;
  std::cout << "h                     displays a synopsis of commands (this screen), and exits" << std::endl;
}