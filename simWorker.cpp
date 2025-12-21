#include "simWorker.h"
#include "constants.h"

#include <QThread>
#include <iostream>
#include <chrono>               // for sleep

simWorker::simWorker(pctxT pcontext, psystemT psystem, QObject* p) : QObject(p), m_context(*pcontext), m_system(psystem)
{

}


void simWorker::start()
{
  uint64_t maxSteps = ceil((m_context.duration * 365.25 * 24 * 3600) / m_context.deltaT);

  for (int step = 0; step < maxSteps; step++)
  {
    // (1) calcuate net force on each object in the system, calculating the new position and velocity vectors
    for (uint32_t obj = 0; obj < m_system->cntObjects; obj++)
    {
      double objMass = m_system->objects.at(obj)->mass;                      // current object's mass
      vector<double_t, 3> objPos = m_system->objects.at(obj)->curPos;        // current object's positions

      std::cout << "working on object: " << m_system->objects.at(obj)->name.c_str() << " with mass of " << objMass << " located at: ";
      objPos.printVector(); std::cout << std::endl;

      // iterate over objects array and calculate force on each object
      vector<double_t, 3> netForce; netForce.coord(0.0, 0); netForce.coord(0.0, 1); netForce.coord(0.0, 2);
      for (uint32_t ndx = 0; ndx < m_system->cntObjects; ndx++)
      {
        if (obj == ndx) continue;                                            // no force between object and itself...
        std::cout << "    calculating force of " << m_system->objects.at(ndx)->name.c_str() << " on " << m_system->objects.at(obj)->name.c_str() << std::endl;
        double otherMass = m_system->objects.at(ndx)->mass;
        vector<double_t, 3> otherPos = m_system->objects.at(ndx)->curPos;
        vector<double_t, 3> dist = otherPos - objPos;

        netForce = netForce + (dist * ((G * otherMass*objMass) / (dist.len() * dist.len() * dist.len())));
      }
      std::cout << "    net force on " << m_system->objects.at(obj)->name.c_str() << " is: "; netForce.printVector(); std::cout << std::endl;

      // calculate the new velocity vector. v_{i+1} = v_i + a*t
      m_system->objects.at(obj)->newVel = m_system->objects.at(obj)->curVel + netForce * (1 / objMass) * m_context.deltaT;

      // use updated velocity vector to calcualte new position vector. x_{i+1} = x_i + v_{i+1}*t
      m_system->objects.at(obj)->newPos = m_system->objects.at(obj)->curPos + m_system->objects.at(obj)->newVel * m_context.deltaT;
    }

    // (2) for all ogjects, update current vectors to the new ones.
    for (uint32_t obj = 0; obj < m_system->cntObjects; obj++)
    {
      std::cout << "  new values for " << m_system->objects.at(obj)->name.c_str() << " are: ";
      std::cout << "to       "; m_system->objects.at(obj)->newPos.printVector(); std::cout << ", "; m_system->objects.at(obj)->newVel.printVector();
      std::cout << std::endl;

      m_system->objects.at(obj)->curPos = m_system->objects.at(obj)->newPos;
      m_system->objects.at(obj)->curVel = m_system->objects.at(obj)->newVel;
    }

    // (3) let main form know we are done with one iteration and update display

    
    QThread::msleep(10);          // sleep for ten milliseconds

    emit tick(step);              // update the mainform date control in the status bar.
  }

  std::cout << "emitting finished signal" << std::endl;
  emit finished();
}