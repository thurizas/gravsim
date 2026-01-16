#include "..\vector.h"
#include "..\shiftbuf.h"
#include "..\matrix.h"

void testShiftBuffer();
void testMatrix();

int main(int argc, char** argv)
{
  try
  {
    testMatrix();
  }
  catch (std::exception& exc)
  {
    std::cout << exc.what() << std::endl;
  }


}

void testShiftBuffer()
{
  shiftBuf<int, 10> sb;
  sb.printBuf();

  sb.push(10);
  sb.push(9);
  sb.push(8);
  sb.push(7);
  sb.push(6);
  sb.push(5);
  sb.push(4);
  sb.push(3);
  sb.push(2);
  sb.push(1);

  sb.printBuf();

  sb.push(-1);

  sb.printBuf();
}

void testMatrix()
{
  matrix3<int> ones(1);
  matrix3<int> zeros(0);
  matrix3<int> A;
  matrix3<float> B;
  matrix3<int> C;
  matrix3<int> D;
  matrix3<int> E;

  vector<int, 3> v1; v1.coord(rand(), 0); v1.coord(rand(), 1); v1.coord(rand(), 2);
  vector<int, 3> v2; v2.coord(1, 0); v2.coord(2, 1); v2.coord(3, 2);

  for (int ndx = 0; ndx < 9; ndx++)
  {
    A.setElem(ndx, ndx);
    B.setElem(ndx, static_cast<float>(rand())/static_cast<float>(RAND_MAX));
    C.setElem(ndx, -ndx);
    D.setElem(ndx, ndx % 5 + 4);
  }

  E = A;  

  std::cout << "ones matris is: " << ones << std::endl;
  std::cout << "zero matrix is: " << zeros << std::endl;
  C.id();
  std::cout << "the identity matrix is: " << C << std::endl;
  std::cout << "matrix A is: " << A << std::endl;
  std::cout << "matrix B is: " << B << std::endl;
  std::cout << "matrix D is: " << D << std::endl;
  std::cout << "matrix E is: " << E << std::endl;
  std::cout << "vector 1 is: " << v1 << std::endl;
  std::cout << "vector 2 is: " << v2 << std::endl;

  std::cout << "A == D : " << (A == D ? "true" : "false") << std::endl;
  std::cout << "A == E : " << (A == E ? "true" : "false") << std::endl;

  std::cout << "A + D is: " << (A + D) << std::endl;
  std::cout << "A - D is: " << (A - D)<< std::endl;
  std::cout << "D - A is: " << (D - A)<< std::endl;
  std::cout << "10*A is: " << (A * 10) << std::endl;
  std::cout << "A*D is: " << (A * D)<< std::endl;
  std::cout << "trace A is: " << A.trace() << std::endl;
  std::cout << "transpose A is: " << A.transpose() << std::endl;
  std::cout << "determinate of B is: " << B.det() << std::endl;
  std::cout << "adjoint of B is: " << B.adjoint() << std::endl;
  std::cout << "inverse of B is: " << B.inverse() << std::endl;

  std::cout << "the product A * v1 is: " << A * v1 << std::endl;
  std::cout << "the product v2*A is: " << v2 * A << std::endl;

}
