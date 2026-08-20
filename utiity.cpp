#include "constants.h"
#include "common.h"
#include "utility.h"
#include "libprocgen/vector.h"

#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const struct _orbpro& op)
{
  os << std::setw(10) /* << op.n << ",\t" */<< std::setprecision(6) << op.a << ",\t" << op.e << ",\t" << op.i << ",\t" <<
                         op.L << ",\t" << op.w << ",\t" << op.W << ",\t" /* << op.m*/ << ",\t" << op.t << ",\t" << op.o << std::endl;


  return os;
}


std::ostream& operator<<(std::ostream& os, const struct system& s)
{
  os << "system properties\nprimary: " << std::endl;
  os << "    mass: " << s.primary.mass << " solar masses" << std::endl;
  os << "    inner system limit: " << s.primary.isl << " AU" << std::endl;
  os << "    outer system limit: " << s.primary.osl << " AU" << std::endl;
  os << "orbital objects: (" << s.cntObjects-1 << ") " << std::endl;

  for (auto o : s.objects)
  {
    os << o->ndx << ", \t" << std::setw(7) << std::setprecision(4) << o->name << ", \t" << o->mass << ", \t" ;
    o->curPos.printVector(); std::cout << " "; o->curVel.printVector();
    os << std::endl;
  }
 
  return os;
}

std::ostream& operator<<(std::ostream& os, const struct renderInfo& s)
{
  os << "    [" << s.ndx << "]";
  os << s.name.c_str();
  os << ",mass:  " << s.mass;
  os << ", position" << s.pos;
  return os;
}

// function to calculate star properties...
// mass in solar masses -- return luminosity relative to Sol
double_t calcLum(double_t mass)
{
  float l;
  if (mass < 0.43) l = 0.23 * pow(mass, 2.3);
  else if (mass < 2) l = mass * mass * mass * mass;
  else l = 1.4 * pow(mass, 3.5);

  return l;
}

// mass in solar masses -- returns max age in Gyrs
double_t calcMAge(double_t mass)
{
  float a;
  float l = calcLum(mass);

  a = 10 * mass / l;
  return a;
}

// mass in solar masses, return radius in solar radii
double_t calcRadius(double_t mass)
{
  float r;

  if (mass < 1) r = pow(mass, 0.8);
  else r = pow(mass, 0.57);

  return r;
}

// mass in solar masses, return temperature in degrees K
double_t calcTemp(double_t mass)
{
  float t;
  float r = calcRadius(mass);
  float l = calcLum(mass);

  if (mass < 0.08)  return 1000;             // brown dwarf have surface temperature < 1000K
  if (mass > 90.0)  return 6000;             // red giants have a surface temperature ~4000K, blue giants ~13000K

  t = 5776 * pow((l / (r * r)), 0.25);

  return t;
}

/**********************************************************************************************************************
 *  consider the metric around a spherically symmetric , non-rotating object,
 *      ds^2 = -yc^2dt^2 + y^{-1}dr^2 + r^2dtheta^2 + r^2sin^2(theta)dphi^2
 *  where y is (1 - (2GM/c^2r) with G is the univeral gravitation constant and M is the mass of teh object,
 * and r is the distance from the center of the object.  When y approximately 1 we have the metric,
 *      ds^2 = -c^2dt^2 + dr^2 + r^2dtheta^2 + r^2sin^2(theta)dphi^2
 * which is what we expect for a flat space-time.
 *
 * We solve for an r value by comparing the result of 1 - (GM/c^2r) to 1 and look for an r-value that makes this close
 * to 1 (close to meaning with-in 2.5E-10 difference).  This was choosen to produce and OSL that is close to the semi
 * major axis of Pluto.
 *
 * returns outer system limit in meters
 *
 * mass in solar masses, return distance in AU.
 */
double_t calcOSL(double_t m)
{
  double_t mass = m * MSOL;   
  double_t guess = calcISL(m) * AU;                         // get inital guess in meters
  double_t delta = 0.1 * AU;                                // how much we are going to increase radius each step
  double_t epsilon = 2.5e-10f;

  double_t val = 1.0L - ((G * mass) / (c_vac * c_vac * guess));
  double_t diff = 0.0L;
  do
  {
    guess += delta;                                        // increment guess by 0.1 AU

    val = 1.0L - ((G * mass) / (c_vac * c_vac * guess));
    diff = 1.0L - val;

  } while (fabs(1.0L - val) > epsilon);

  return guess;
}

// mass in solar masses, minHZ and maxHZ returned in AU
void calcHZ(double_t mass, double_t* minHZ, double_t* maxHZ)
{
  float l = calcLum(mass);
  *minHZ = sqrt(l / 1.1);
  *maxHZ = sqrt(l / 0.53);
}


// calculate roche limit for star
// mass in solar units, return value in AU.
double_t calcISL(double_t mass)
{
  float radius = calcRadius(mass);
  float density = mass / powf(radius, 3);
  float isl = 2.455 * (RSOL * radius) * powf((density * DSOL) / 5400, 1.0f / 3) / AU2KM;

  return isl;
}

// mass in solar masses, return frost line in AU
double_t calcFrostLine(double_t mass)
{
  float lumin = calcLum(mass);
  float fl = 4.85 * sqrt(lumin);

  return fl;
}

/**********************************************************************************************************************
 * Function:   
 *
 * Abstract:                         volume of shell-slice = f(x^*)(\pi r_o^2 - \pi r_i^2)     base area (circular annali) time height
 *                                                         = f(x^*)\pi(r_o - r_i)(r_o + r_i)
 *                                                         = 2f(x^*)\frac{r_o + r_i}{2}(r_o - r_i)
 *                           
 *                                       note: r_o - r_i = \Delta x and \frac{r_o + r_i}{2} \appro x^* thus
 *                                           
 *                                   volume of shell-slice \appro 2\pif(x^*)x^* \Delta x = \int_{r_i}^{r_o} 2\pi f(x) x dx
 * 
 *             shell:
 *     /| -------------------------  for the exocone, f(x) = x \tan(\theta) where \theta is the vertex angle
 *    / |                         |                       
 *   /  |                         |  volume of cone is V = 2 \pi \tan(\theta) \int_{r_o}^{r_i} x^2 dx  (upper half)                      
 *  /   |                         |                      = \frac{2\pi \tan(theta}{3} x^3|_{x_o}^{x_i}
 * /    | ______                  |                      = \frac{2\pi \tan(theta}{3}(x_o^3 - x_i^3)
 * |    |   |                     |                      = \frac{4\pi \tan(theta}{3}(x_o^3 - x_i^3)    (upper & lower parts)
 * |    |   |                     |   r-o\tan(\theta)
 * |    |   |    r_i\tan(\theta)  |
 * |    |   |                     |
 * |    |   |                     |  volume of shell = \pi \int_{r_i}^{r_o} (r_i\tan(\theta) + r_o\tan(\theta))dr
 * |    |   |                     |                  = \pi(r_i\tan(\theta) + r_o\tan(\theta))*\frac{1}{2}r^2 |_r_i^r_o = \pi(r_i\tan(\theta) + r_o\tan(\theta)(r_o^2 -r_i^2)
 * +----+ -------------------------                   
 *r_i  r_o     
 *
 *                                   funstrum
 * Input   : angle -- [in] the vertex angle of the exocone, in degrees
 *           inner -- [in] the inner radius of the section, in AU
 *           outer -- [in] the outer radius of the section, in AU
 *
 * Returns : volume of the slice (a trapezoid rotated around the z-axis) in AU^3
 *
 * Written : Jan 2026 (gkhuber) 
 *************************************************************************************************/
double_t exoconeVolume(double_t angle, double_t inner, double_t outer)
{
  double_t    dispAngle = angle * DEG2RAD;                  // convert degrees to radians

  double_t volumn = ((4.0*PI * tan(dispAngle))/3.0) * (pow(outer, 3) - pow(inner, 3));

  return volumn;
}
