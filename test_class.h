
#ifndef TEST_CLASS_
#define TEST_CLASS_

#include <iostream.h>

class TestClass {
public:
  TestClass(const std::string& name) : name_(name) { 
    std::cout << "TestClass [ " << name_ << " ] ctor" << std::endl; 
  }
  ~TestClass() { 
    std::cout << "TestClass [ " << name_ << " ] dtor" << std::endl;
  }
  
  TestClass(const TestClass& other) { 
    this->name_ = other.name_ + " /copy/"; 
    std::cout << "TestClass [ " << name_ << " ] copy ctor" << std::endl; 
  }
  
  TestClass(TestClass&& other) { 
    this->name_ = other.name_ + " /moved in/"; 
    other.name_ += " /moved out/";
    std::cout << "TestClass [ " << name_ << " ] move ctor" << std::endl;
  }
  
  TestClass& operator=(const TestClass& other) {
    this->name_ = other.name_;
    std::cout << "TestClass [ " << name_ << " ] assignment" << std::endl;
    return *this;
  }
  
  TestClass& operator=(TestClass&& other) {
    this->name_ = other.name_;
    other.name_ += " /assigned out/";
    std::cout << "TestClass [ " << name_ << " ] move assignment" << std::endl;
    return *this;
  }
  
private:
  std::string name_;
};

#endif
