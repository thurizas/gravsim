#ifndef _common_h_
#define _common_h_

#include "libprocgen/vector.h"

#include <string>
#include <vector>

typedef struct ctx
{
  double      start;          // date to start the simualtion at
  double      deltaT;         // time step for integration, in seconds
  double      duration;       // years to run simulation for
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
    vector<double_t, 3> newPos;
    vector<double_t, 3> newVel;
  } objectT, *pobjectT;

  uint8_t               method;
  uint32_t              cntObjects;
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
  char     n[10];         // name of the object

  friend std::ostream& operator<<(std::ostream&, const struct _orbpro&);
} orbitalPropT, * porbitalPropT;



#endif
