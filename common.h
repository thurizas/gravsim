#ifndef _common_h_
#define _common_h_

#include "libprocgen/vector.h"
#include "libprocgen/shiftBuf.h"

#include <QColor>

#include <string>
#include <vector>

// define colors for upto nine planets and primary
static QColor colors[] = { QColor(0xFF,0xFF,0x00), QColor(0x60,0x60,0x60), QColor(0xCC,0xCC,0x00), QColor(0x00,0x00,0xCC), QColor(0xFF,0xB2,0x66), QColor(0xF0,0x03,0xFC),
                           QColor(0x03,0xFC,0xB3), QColor(0xFF,0x00,0x00), QColor(0xFF,0xFF,0x99), QColor(0x00,0x80,0xFF), QColor(0x00,0x00,0x99) };


typedef struct ctx
{
  double      start;          // date to start the simualtion at
  double      deltaT;         // time step for integration, in seconds
  double      duration;       // time to run simulation for
  std::string durUnits;       // units for the duration
  bool        showOrbits;     // flag to draw orbits or not.
  uint8_t     method;         // method of getting system info
  std::string datafile;       // file containing input data
} ctxT, *pctxT;

typedef struct system
{
  enum method: std::uint8_t{ACCRETE=0, MANUAL=1};

  typedef struct star
  {
    double_t     mass;
    double_t     isl;
    double_t     osl;
  } starT, *pstarT;

  typedef struct obj
  {
    uint32_t            ndx;
    double_t            mass;
    std::string         name;
    vector<double_t, 3> curPos;
    vector<double_t, 3> curVel;
    vector<double_t, 3> netForce;
    vector<double_t, 3> newPos;
    vector<double_t, 3> newVel;
    //std::vector<vector<double_t, 3>> orbit;
    shiftBuf<vector<double_t, 3>, 45> orbit;
  } objectT, *pobjectT;

  uint8_t               method;
  uint32_t              cntObjects;
  uint64_t              timeStep;
  std::vector<objectT*> objects;
  starT                 primary;

  friend std::ostream& operator<<(std::ostream&, const struct system&);
} systemT, *psystemT;

typedef struct _orbpro
{
  double   a;             // semi-major axis, AU
  double   e;             // eccentricity of orbit
  double   i;             // angle of inclination of orbit, degrees
  double   L;             // mean longitude, degrees
  double   w;             // longitude of perihelion, degrees
  double   W;             // longitude of ascending node, degrees
  double   m;             // mass of the planet, earth masses
  double   t;             // orbital period, days
  double   o;             // obliquity of ecliptic, degrees
  double   s;             // linear speed, km/s
  char     n[10];         // name of the object

  friend std::ostream& operator<<(std::ostream&, const struct _orbpro&);
} orbitalPropT, * porbitalPropT;



#endif
