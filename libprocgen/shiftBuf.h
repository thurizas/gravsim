#ifndef _shiftBuf_h_
#define _shiftBuf_h_

#include "procgen_priv.h"


#include <array>
#include <cstddef>
#include <stdexcept>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>


template <typename T, std::size_t N>
class DLLExport shiftBuf
{
public:
  shiftBuf() 
  {
  
  }
  ~shiftBuf() {}

  int32_t count() { return m_currentItems; }

  void push(T item)
  {
    if (m_currentItems == 0)                 // buffer is empty
    {
      m_data[0] = item;

    }
    else
    {
      // move all items up one space
      for (int32_t ndx = m_currentItems-1; 0 <= ndx; ndx--)
      {
        if (ndx + 1 < m_size)
          m_data[ndx + 1] = m_data[ndx];
      }

      m_data[0] = item;
    }
    
    m_currentItems += 1;
    if (m_currentItems > m_size) m_currentItems -= 1;
  }

  T at(int32_t ndx)
  {
    return m_data.at(ndx);
  }

  std::vector<T> data()
  {
    std::vector<T> ret;

    for (int32_t n = 0; n < m_currentItems-1; n++)
    {
      ret.push_back(m_data[n]);
    }

    return ret;
  }

  T peek()
  {
    return m_data[0];
  }

  void printBuf()
  {
    for (T i : m_data)
    {
      std::cout << i << ", ";
    }

    std::cout << std::endl;
  }

private:
  std::size_t      m_size = N;
  int32_t          m_currentItems = 0;
  std::array<T, N> m_data = { {0} };
};

#endif