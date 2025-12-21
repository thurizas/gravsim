#include "constants.h"
#include "common.h"
#include "libprocgen/vector.h"

#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const struct _orbpro& op)
{
  os << std::setw(10) << op.n << ",\t" << std::setprecision(6) << op.a << ",\t" << op.e << ",\t" << op.i << ",\t" <<
                         op.L << ",\t" << op.w << ",\t" << op.W << ",\t" << op.m << ",\t" << op.t << ",\t" << op.o << std::endl;


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