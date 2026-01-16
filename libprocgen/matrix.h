#ifndef _matrix_h_
#define _matrix_h_

#include "procgen_priv.h"

#include <cstddef>
#include <cstdio>
#include <stdexcept>

template <typename T>
class DLLExport matrix3
{
public:
  matrix3() : m_rows(3), m_cols(3) 
  {
    m_data = new T[3 * 3];
  }

  matrix3(T n) : m_rows(3), m_cols(3)
  {
    m_data = new T[3 * 3];
    for (uint32_t ndx = 0; ndx < m_rows*m_cols; ndx++)
    {
      m_data[ndx] = n;
    }

  }

  // copy constructor
  matrix3(const matrix3<T>& other)
  {
    m_rows = other.m_rows;
    m_cols = other.m_cols;

    m_data = new T[m_rows * m_cols];
    for (uint32_t ndx = 0; ndx < m_rows * m_cols; ndx++)
    {
      m_data[ndx] = other.m_data[ndx];
    }
  }

  // assignment operator
  matrix3<T> operator=(matrix3<T> other)
  {
    if (&other != this)
    {
      m_rows = other.m_rows ;
      m_cols = other.m_cols;

      m_data = new T[m_rows * m_cols];
      for (uint32_t ndx = 0; ndx < m_rows * m_cols; ndx++)
        m_data[ndx] = other.m_data[ndx];
    }
    return *this;
  }
  ~matrix3()
  {
    if (nullptr != m_data)
      delete[] m_data;
  }

  void clear()
  {
    for (uint32_t ndx = 0; ndx < 9; ndx++)
      m_data[ndx] = (T)0;
  }

  T a00() { return m_data[0]; }
  T a01() { return m_data[1]; }
  T a02() { return m_data[2]; }
  T a10() { return m_data[3]; }
  T a11() { return m_data[4]; }
  T a12() { return m_data[5]; }
  T a20() { return m_data[6]; }
  T a21() { return m_data[7]; }
  T a22() { return m_data[8]; }

  void setElem(uint32_t ndx, T val)
  {
    if (ndx < m_rows * m_cols)
    {
      m_data[ndx] = val;
    }
    else
    {
      throw std::exception("index out of bounds.");
    }
  }

  matrix3<T> id()                                 // convert matrix into identity matrix
  {
    this->clear();
    for(uint32_t i = 0; i < m_cols; i++)
    {
      m_data[mkindex(i, i)] = (T)1;
    }

    return *this;
  }

  bool operator==(matrix3<T>& other)
  {
    bool ret = false;

    if ((m_rows == other.m_rows) && (m_cols == other.m_cols))
    {
      ret = true;

      for (uint32_t ndx = 0; ndx < m_rows * m_cols; ndx++)
      {
        if (m_data[ndx] != other.m_data[ndx])                      // TODO : handle the case were T := float_t
        {
          ret = false;
          break;
        }
      }
    }

    return ret;
  }

  matrix3<T> operator+(matrix3<T> other)
  {
    matrix3<T> ret;
    if ((m_rows == other.m_rows) && (m_cols == other.m_cols))
    {

      for (uint32_t ndx = 0; ndx < m_rows * m_cols; ndx++)
      {
        ret.m_data[ndx] = m_data[ndx] + other.m_data[ndx];
      }
    }
    else
    {
      throw std::exception("addition - matricies must be same size");
    }

    return ret;
  }

  matrix3<T> operator-(matrix3<T> other)
  {
    matrix3<T> ret;
    if ((m_rows == other.m_rows) && (m_cols == other.m_cols))
    {

      for (uint32_t ndx = 0; ndx < m_rows * m_cols; ndx++)
      {
        ret.m_data[ndx] = m_data[ndx] - other.m_data[ndx];
      }
    }
    else
    {
      throw std::exception("subtraction - matricies must be same size");
    }

    return ret;
  }

  matrix3<T> operator* (T scalar)
  {
    matrix3<T> ret;

    for (uint32_t ndx = 0; ndx < m_rows * m_cols; ndx++)
    {
      ret.m_data[ndx] = scalar * m_data[ndx];
    }

    return ret;
  }



  // C_{in} = A_{ij} * B_{mn}    c_in = a_{ik} * b_{kn}
  matrix3<T> operator*(matrix3<T> other)
  {
    matrix3<T> ret(0);

    if ((m_cols == other.m_rows))
    {
      for (uint32_t k = 0; k < m_rows; k++)
        for (uint32_t i = 0; i < m_cols; i++)
          for (uint32_t n = 0; n < m_rows; n++)
            ret.m_data[mkindex(i, n)] += m_data[mkindex(i, k)] * other.m_data[mkindex(k, n)];
    }
    else
    {
      throw std::exception("multiplication -- matricies are mis-matched in size");
    }

    return ret;
  }

  vector<T, 3> operator*(vector<T, 3> other)
  {
    vector<T, 3> ret;

    T x = a00() * other.coord(0) + a01() * other.coord(1) + a02() * other.coord(2);
    T y = a10() * other.coord(0) + a11() * other.coord(1) + a12() * other.coord(2);
    T z = a20() * other.coord(0) + a21() * other.coord(1) + a22() * other.coord(2);

    ret.coord(x, 0);
    ret.coord(y, 1);
    ret.coord(z, 2);

    return ret;
  }

  

  /*
     inverse is calculated by transpose(cofactor(A))/det(A)
  */
  matrix3<T> inverse()
  {
    matrix3<T> ret;
    T det = this->det();
    if (det != 0)               // is matrix invertible
    {
      ret = adjoint() * (1 / det);
    }

    return ret;
  }

  matrix3<T> adjoint()
  {
    matrix3<T> ret;
    matrix3<T> cf = cofactors();
    ret = cf.transpose();
    return ret;
  }
 
  matrix3<T> cofactors()
  {
    matrix3<T> ret;

    ret.setElem(0, a11() * a22() - a12() * a21());
    ret.setElem(1, -(a10() * a22() - a12() * a20()));
    ret.setElem(2, a10() * a21() - a11() * a20());
    ret.setElem(3, -(a01() * a22() - a02() * a21()));
    ret.setElem(4, a00() * a22() - a02() * a20());
    ret.setElem(5, -(a00() * a21() - a01() * a20()));
    ret.setElem(6, a01() * a12() - a02() * a11());
    ret.setElem(7, -(a00() * a12() - a02() * a10()));
    ret.setElem(8, a00() * a11() - a01() * a10());

    return ret;
  }


  matrix3<T> transpose()
  {
    matrix3<T> ret = *this;

    for(uint32_t r = 0; r < m_rows; r++)
    {
      for(uint32_t c = 0; c < r; c++)
      {
        T temp = ret.m_data[mkindex(r, c)];
        ret.m_data[mkindex(r, c)] = ret.m_data[mkindex(c, r)];
        ret.m_data[mkindex(c, r)] = temp;
      }
    }

    return ret;
  }

  T det()
  {
    T m00 = m_data[mkindex(0,0)];
    T m01 = m_data[mkindex(0,1)];
    T m02 = m_data[mkindex(0,2)];

    T detM00 = m_data[mkindex(1, 1)] * m_data[mkindex(2, 2)] - m_data[mkindex(1, 2)] * m_data[mkindex(2, 1)];
    T detM01 = m_data[mkindex(1, 0)] * m_data[mkindex(2, 2)] - m_data[mkindex(1, 2)] * m_data[mkindex(2, 0)];
    T detM02 = m_data[mkindex(1, 0)] * m_data[mkindex(2, 1)] - m_data[mkindex(1, 1)] * m_data[mkindex(2, 0)];

    T det = m00 * detM00 - m01 * detM01 + m02 * detM02;
    return det;
  }

  T trace()
  {
    T sum = (T)0;

    for (uint32_t r = 0; r < m_rows; r++)
      sum += m_data[mkindex(r, r)];

    return sum;
  }

  void printMatrix()
  {
    std::cout << "[";
    for (uint32_t r = 0; r < m_rows; r++)
    {
      for (uint32_t c = 0; c < m_cols; c++)
      {
        std::cout << " " << m_data[r * m_cols + c];
      }
      std::cout << ";";
    }
    std::cout << "]";
  }

  
  friend std::ostream& operator<<(std::ostream& os, const matrix3<T>& other)
  {
    os << "[";
    for (uint32_t r = 0; r < other.m_rows; r++)
    {
      for (uint32_t c = 0; c < other.m_cols; c++)
      {
        os << " " << other.m_data[r*other.m_cols + c];
      }
      os << ";";
    }
    os << "]";

    return os;
  }

private:
  uint32_t  m_rows;   // number of rows
  uint32_t  m_cols;   // number of columns
  T*        m_data;   // data in the matrix, row-major format

  uint32_t mkindex(uint32_t r, uint32_t c) { return r * m_cols + c; }

};

template<typename T>
vector<T, 3> operator*(vector<T, 3> lh, matrix3<T> rh)
{
  vector<T, 3> ret;

  T x = lh.coord(0);
  T y = lh.coord(1);
  T z = lh.coord(1);

  ret.coord(rh.a00() * lh.coord(0) + rh.a10() * lh.coord(1) + rh.a20() * lh.coord(2), 0);
  ret.coord(rh.a01() * lh.coord(0) + rh.a11() * lh.coord(1) + rh.a21() * lh.coord(2), 1);
  ret.coord(rh.a02() * lh.coord(0) + rh.a12() * lh.coord(1) + rh.a22() * lh.coord(2), 2);

  return ret;
}


#endif
