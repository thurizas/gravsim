#ifndef _utility_h_
#define _utility_h_

double_t calcLum(double_t mass);
double_t calcMAge(double_t mass);
double_t calcRadius(double_t mass);
double_t calcTemp(double_t mass);
double_t calcOSL(double_t m);
void     calcHZ(double_t mass, double_t* minHZ, double_t* maxHZ);
double_t calcISL(double_t mass);
double_t calcFrostLine(double_t mass);

double_t exoconeVolume(double_t, double_t, double_t);



#endif
