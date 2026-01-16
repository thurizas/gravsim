#ifndef _vector_h_
#define _vector_h_

#include "procgen_priv.h"


#include <array>
#include <cstddef>
#include <stdexcept>
#include <cstdio>
#include <iostream>
#include <string>


template <typename T, std::size_t N>
class DLLExport vector
{
public:
  vector() { }
  vector(int val) : m_size(N) { for (int n = 0; n < m_size; n++) m_coords[n] = (T)val; }
  vector(const vector& other) 
  {
    this->m_size = other.m_size;
    for (int ndx = 0; ndx < m_size; ndx++)
    {
      this->m_coords[ndx] = other.m_coords[ndx];
    }
  }

  vector operator=(const vector& rhs)
  {
    if (*this == rhs) return *this;

    this->m_size = rhs.m_size;
    for (int ndx = 0; ndx < m_size; ndx++)
    {
      this->m_coords[ndx] = rhs.m_coords[ndx];
    }
    return *this;
  }

  T coord(std::size_t n) { if ((n >= 0) && (n < N)) return m_coords[n]; else { std::out_of_range exc("index out of range"); throw exc; } }
  void coord(T val, std::size_t n) {if((n >=0) && (n<N)) m_coords[n] = val; else{std::out_of_range exc("index out of range"); throw exc; } }
  
  size_t size() { return m_size; }

  // equality overload
  bool operator==(const vector& other)
  {
    bool bret = false;

    if (this->m_size != other.m_size)
      return bret;

    bret = true;
    for(int ndx = 0; ndx < m_size; ndx++)
    {
      if (this->m_coords[ndx] != other.m_coords[ndx])
      {
        bret = false;
        break;
      }
    }

    return bret;
  }

  // addition overload
  vector operator+(const vector& other) 
  {
    vector<T, N> retVec;
    if (this->m_size == other.m_size)
    {

      for (int ndx = 0; ndx < m_size; ndx++)
      {
        retVec.coord(m_coords[ndx] + other.m_coords[ndx], ndx);
      }
    }
    else
    {
      std::out_of_range exc("addition - vectors must be the same size");
      throw exc;
    }

    return retVec;
  }
  
  // subtraction overload
  vector operator-(const vector& other) 
  {
    vector<T, N> retVec;
    if (this->m_size == other.m_size)
    {

      for (int ndx = 0; ndx < m_size; ndx++)
      {
        retVec.coord(m_coords[ndx] - other.m_coords[ndx], ndx);
      }
    }
    else
    {
      std::out_of_range exc("subtraction - vectors must be the same size");
      throw exc;
    }

    return retVec;
  }

  // multiplication overload - handles V*k  (i.e. post multiplication by scalar)
  vector operator*(const T& factor) 
  {
    vector<T, N> retVec;

    for (int ndx = 0; ndx < m_size; ndx++)
    {
      retVec.coord(m_coords[ndx] * factor, ndx);
    }

    return retVec;
  }

  T dot(const vector& other) 
  {
    T retVal = 0;
    if (this->m_size == other.m_size)
    {
      for (int ndx = 0; ndx < m_size; ndx++)
      {
        retVal += m_coords[ndx]*other.m_coords[ndx];
      }
    }
    else
    {
      std::out_of_range exc("subtraction - vectors must be the same size");
      throw exc;
    }

    return retVal;
  }

  vector cross(const vector&) {}
  
  void normalize();

  double len() 
  {
    return sqrt(len2());
  }

  double len2() 
  {
    double sum = 0.0;

    for (int ndx = 0; ndx < m_size; ndx++)
      sum += m_coords[ndx] * m_coords[ndx];

    return sum;
  }

  void printVector()
  {
    std::cout << "[";
    for (size_t ndx = 0; ndx < m_size; ndx++)
    {
      if (ndx < m_size - 1)
        std::cout << " " << m_coords[ndx] << ",";
      else
        std::cout << " " << m_coords[ndx] << "]";
    }
  }

  // f:  conversion factor to convert units to AU
  std::string toString(double f = 1.0f)
  {
    std::string    ret("[");

    for (size_t ndx = 0; ndx < m_size - 1; ndx++)
    {
      ret += std::to_string(m_coords[ndx] / f) + ", ";
    }

    ret += std::to_string(m_coords[m_size - 1] / f) + "]";

    return ret;
  }

  friend std::ostream& operator<<(std::ostream& os, const vector<T, N>& other)
  {
    vector<T, N> vec = const_cast<vector<T, N>&>(other);
    os << "[";
    for (uint32_t ndx = 0; ndx < vec.size(); ndx++)
      os << vec.coord(ndx) <<", ";
    os << "]";

    return os;
  }

private:
  std::size_t m_size = N;
  std::array<T, N> m_coords{ 0 };
};




#endif