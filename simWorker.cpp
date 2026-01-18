#include "simWorker.h"
#include "constants.h"

#include <QThread>
#include <iostream>
//#include <chrono>               // for sleep
#include <QApplication>

simWorker::simWorker(pctxT pcontext, psystemT psystem, QObject* p) : QObject(p), m_context(*pcontext), m_system(psystem)
{
  // account for different units in duration  "seconds", "minutes", "hours", "days", "months(30 day)", "years"
  double_t factor = 1.0;
  std::string units = m_context.durUnits;
  if (units == "seconds") factor = 1.0;
  else if (units == "minutes") factor = 60.0;
  else if (units == "hours") factor = 3600.0;
  else if (units == "days") factor = 86400.0;
  else if (units == "months(30 day)") factor = 30.0 * 86400.0;
  else factor = 365.25 * 86400;
  uint64_t m_maxSteps = ceil((m_context.duration * factor) / m_context.deltaT);

  m_bRunning = true;
}


void simWorker::start()
{
  // account for different units in duration  "seconds", "minutes", "hours", "days", "months(30 day)", "years"
  double_t factor = 1.0;
  std::string units = m_context.durUnits;
  if (units == "seconds") factor = 1.0;
  else if (units == "minutes") factor = 60.0;
  else if (units == "hours") factor = 3600.0;
  else if (units == "days") factor = 86400.0;
  else if (units == "months(30 day)") factor = 30.0 * 86400.0;
  else factor = 365.25 * 86400;
  uint64_t m_maxSteps = ceil((m_context.duration * factor) / m_context.deltaT);

  m_bRunning = true;

  while (m_bRunning)
  {
    while (!m_bPaused)
    {
      int step = 0;
      emit updateStatusBar("Simulation Running");

      for (step = 0; step < m_maxSteps; step++)
      {
        QGuiApplication::processEvents(QEventLoop::AllEvents);

        renderInfoT* info = new renderInfoT[m_system->cntObjects];

        m_context.systemMutex->lock();
        m_system->timeStep = step;

        // (1) calcuate net force on each object in the system, calculating the new position and velocity vectors
        for (uint32_t obj = 0; obj < m_system->cntObjects; obj++)
        {
          vector<double_t, 3> netForce; netForce.coord(0.0, 0); netForce.coord(0.0, 1); netForce.coord(0.0, 2);

          // iterate over objects array and calculate force on each object
          for (uint32_t ndx = 0; ndx < m_system->cntObjects; ndx++)
          {
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
          info[m_system->objects.at(obj)->ndx] = renderInfo{ m_system->objects.at(obj)->ndx, m_system->objects.at(obj)->name,
                                                             m_system->objects.at(obj)->mass, m_system->objects.at(obj)->curPos };
        }
        m_context.systemMutex->unlock();

        // (3) update GUI thread
        emit updateGuiThread(step, info);
        QGuiApplication::processEvents(QEventLoop::AllEvents);
        QThread::usleep(10);                                            // for blocking for 10 usec, to allow events to run



      } // closes for(step ...) loop ......

      if (step == m_maxSteps)                             // done with the simulation
      {
        m_bRunning = false;
        break;
      }
    } // closes while(!m_bPaused) loop....
  } // closes while(m_bRunning) loop....

  emit updateStatusBar("simulation finished");

}