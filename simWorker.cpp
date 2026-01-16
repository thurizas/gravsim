#include "simWorker.h"
#include "constants.h"

#include <QThread>
#include <iostream>
//#include <chrono>               // for sleep
#include <QApplication>

simWorker::simWorker(pctxT pcontext, psystemT psystem, QObject* p) : QObject(p), m_context(*pcontext), m_system(psystem)
{

}


struct stepValues
{
  int64_t            step;
  vector<double_t, 3> origPos;
  vector<double_t, 3> origVel;
  vector<double_t, 3> netForce;
  vector<double_t, 3> newVel;
  vector<double_t, 3> newPos;
};

void simWorker::start()
{
  //uint64_t maxSteps = ceil((m_context.duration * 365.25 * 24 * 3600) / m_context.deltaT);
  uint64_t maxSteps = 40;
  std::map<std::string, std::vector<struct stepValues>>         orbits;

  for (int step = 0; step < maxSteps; step++)
  {
    // (0) lock access to m_system...

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
      m_system->objects.at(obj)->newPos = m_system->objects.at(obj)->curPos + m_system->objects.at(obj)->newVel * (1000*m_context.deltaT/AU);


    } // end obj loop
    
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

    // (3) let main form know we are done with one iteration and let message pump run
    fprintf(stderr, "letting main form know a new calc is ready - step %d\n", step);
    emit tick(step);                                       // update the main GUI.
    QApplication::processEvents(QEventLoop::AllEvents);    // let any pending event run

    m_system->timeStep++;
  } // end time loop

  std::cout << "emitting finished signal" << std::endl;
  emit finished();
}