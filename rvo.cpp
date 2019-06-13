
#include <iostream.h>

#include "test_class.h"

// No RVO is performed because compiler does not know in advance 
// which of the x or y is going to be returned. So compiler creates
// both and then moves out the proper one.
TestClass rvo1(const int i) {
  TestClass x{"from rvo1, x"};
  TestClass y{"from rvo1, y"};
  if (i == 12) {
    return x; 
  } else {
    return y; 
  }
}

// In this case compiler does not know whether x will be returned so
// x is created anyway and then moved out if necessasy. But if the other 
// path is selected then temporary object is returned instead and 
// compiler definitely knows where to place that temp object when 
// it is conctructed, and that knowledge is used to avoid copy/move.
TestClass rvo2(const int i) {
  TestClass x{"from rvo2, x"};
  if (i == 12) {
    return x; 
  } else {
    return TestClass{"from rvo2, temp"}; 
  }
}

// Like prevoius but RVO is performed for both paths.
TestClass rvo3(const int i) {
  if (i == 12) {
    return TestClass{"from rvo2, temp 1"}; 
  } else {
    return TestClass{"from rvo2, temp 2"}; 
  }
}

int main() {
  std::cout << "##### start" << std::endl;
  {
    std::cout << "\n--- NO RVO, case #1" << std::endl;
    auto c{rvo1(12)};
  }
  {
    std::cout << "\n--- NO RVO, case #2" << std::endl;
    auto c{rvo1(0)};
  }
  {
    std::cout << "\n--- Partial RVO, case #1" << std::endl;
    auto c{rvo2(12)};
  }
  {
    std::cout << "\n--- Partial RVO, case #2" << std::endl;
    auto c{rvo2(0)};
  }
  {
    std::cout << "\n--- Full RVO, case #1" << std::endl;
    auto c{rvo3(12)};
  }
  {
    std::cout << "\n--- Full RVO, case #2" << std::endl;
    auto c{rvo3(0)};
  }
  std::cout << "##### end\n" << std::endl;
}