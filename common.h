#ifndef _common_h_
#define _common_h_

#include "libprocgen/vector.h"
#include "libprocgen/shiftBuf.h"

#include <QColor>
#include <QMutex>

#include <string>
#include <vector>

// define colors for upto nine planets and primary
static QColor colors[] = { QColor(0xFF,0xFF,0x00), QColor(0x60,0x60,0x60), QColor(0xCC,0xCC,0x00), QColor(0x00,0x00,0xCC), QColor(0xFF,0xB2,0x66), QColor(0xF0,0x03,0xFC),
                           QColor(0x03,0xFC,0xB3), QColor(0xFF,0x00,0x00), QColor(0xFF,0xFF,0x99), QColor(0x00,0x80,0xFF), QColor(0x00,0x00,0x99) };

/*********************************************************************************************************************
 * structure: accrete_variables
 *
 * abstract : contains the paramterd necessary for the acrete simulation to work.  It is created in the main function
 *            with the default values set.  It is modified in the following places:
 *             (1) in the function mainWnd::onSimConfig
 *            
 *********************************************************************************************************************/
typedef struct accrete_variables
{
  double_t     dispAngle;    // defines height of disk, degrees                   [default = 20.0 ]
  double_t     minMass;      // define minimum mass of disk, percent solar mass   [default = 0.001]
  double_t     avgMass;      // define average mass of disk, percent solar mass   [default = 0.01 ]
  double_t     maxMass;      // define maximum mass of disk, percent solar maas   [default = 0.1  ]
  double_t     gasDustRatio; // ratio of gas mass to dust mass                    [default = 50.0 ]
  uint32_t     cntBands;     // number of 'bands' in disk                         [default = 30   ]
  uint32_t     cntNuclei;    // number of planetary nuclei to start               [default = 20   ]
  double_t     initialMass;  // initial mass of each planetary nuclei, solar mass [default = 10E-15]
  double_t     alpha;        // [default = 5.0]
  double_t     n;            // [default = 3.0]

} accreteVars, *paccreteVars;


/**********************************************************************************************************************
 * structure: ctx
 * 
 * abstract : contains the parameters that define the behavior of the simulation.  This structure is initialized in various 
 *            places:
 *             (1) it is instantiated in 'main' with the default values
 *             (2) datafile and method _may_be set during processing of command line arguments
 *             (3) systemMutex is instantiated during construction of mainWnd
 *             (4) configuration can be view/modified in mainWnd::onSimConfig
 *********************************************************************************************************************/
typedef struct ctx
{
  double      start;           // date to start the simualtion at,         [default = 0]
  double      deltaT;          // time step for integration, in seconds    [default = 86400]
  double      duration;        // time to run simulation for               [default = 400]
  std::string durUnits;        // units for the duration                   [default = "days"
  bool        showOrbits;      // flag to draw orbits or not.              [default = true]
  uint8_t     method;          // method of getting system info            [default = system::method::UNKNOWN]
  uint8_t     earthLike;       // shooting of an earth-like planet         [default = system::earthlike::UNKNOWN]
  std::string datafile;        // file containing input data               [default = ""]
  std::string stellarDataFile; // file containing primary data             [default = ""]
  QMutex*     systemMutex;     // mutex for controlling access to sim data [default = nullptr]
  accreteVars accreteCtx;      // variables controlling the accrete process

} ctxT, *pctxT;


/**********************************************************************************************************************
 * structure: _orbpro
 * 
 * abstract : contains the Keplerian properties of the orbit as well as other physical properties of the object, this
 *            is instanticated at the same time as the objects are created (in onReadSystem or onGenSystem).  The 
 *            orbital properties are used to help set the intial position and velocity vectorw.
*/
typedef struct _orbpro
{
  double   a;             // semi-major axis, AU
  double   e;             // eccentricity of orbit
  double   i;             // angle of inclination of orbit, degrees
  double   L;             // mean longitude, degrees
  double   w;             // longitude of perihelion, degrees
  double   W;             // longitude of ascending node, degrees
  double   t;             // orbital period, days
  double   o;             // obliquity of ecliptic, degrees
  double   s;             // linear speed, km/s

  friend std::ostream& operator<<(std::ostream&, const struct _orbpro&);
} orbitalPropT, * porbitalPropT;


/**********************************************************************************************************************
 * structure: system
 * 
 * abstract : this contains all the information needed to perform the simulation of the planetary motion.  This data 
 *            structure is shared beween the physics thread and the main GUI thread.  the mutex ctx.systemMutex is used
 *            to control access between the two threads.  This structure is created and populated in either the function
 *            onReadSystem or onGenSystem
 *********************************************************************************************************************/
typedef struct system
{
  enum method: std::uint8_t{UNKNOWN = 0, ACCRETE=1, MANUAL=2};
  enum earthlike: std::uint8_t{INDETERMINATE = 0, NO=1, YES=2};

  typedef struct star
  {
    bool         earthLike; // flag to force an earth-like environment
    double_t     mass;      // mass in terms of sol masses
    double_t     age;       // age in millions of year
    double_t     maxage;    // max age in millions of years \tau_{ms} = 10^{10}(\frac{M}{M_sol})^{-2.5}
    double_t     radius;    // radius in km
    double_t     lumins;    // luminosity      
    double_t     temp;      // temperature in kelvins
    double_t     isl;       // inner system limit, AU
    double_t     osl;       // outer system limit, AU
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
    orbitalPropT        orbProps;
    shiftBuf<vector<double_t, 3>, 45> orbit;
  } objectT, *pobjectT;

  uint8_t               method;
  uint32_t              cntObjects;
  uint64_t              timeStep;
  std::vector<objectT*> objects;
  starT                 primary;

  friend std::ostream& operator<<(std::ostream&, const struct system&);
} systemT, *psystemT;




/**********************************************************************************************************************
 * structure: renderInfo
 *
 * abstract : contains the minimum about of informatin necessary to render the amimation.  These object are created in 
 *            the render thread and comsumed in the GUI thread
*/
typedef struct renderInfo
{
  uint32_t     ndx;
  std::string  name;
  double_t     mass;
  vector<double_t, 3> pos;

  friend std::ostream& operator<<(std::ostream&, const struct renderInfo&);
} renderInfoT, *prenderInfoT;



#endif
