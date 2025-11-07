#include "OpenACCParser.h"
#include <cassert>
#include <iostream>

int main() {
  // C-prefixed pragma
  std::string cpragma = "#pragma acc parallel";
  OpenACCDirective *d1 = parseOpenACC(cpragma);
  if (d1 == nullptr) {
    std::cerr << "parseOpenACC returned NULL for C pragma\n";
    return 2;
  }
  if (d1->getBaseLang() != ACC_Lang_C) {
    std::cerr << "Expected ACC_Lang_C but got " << d1->getBaseLang() << "\n";
    return 3;
  }

  // Fortran-prefixed pragma
  std::string fpragma = "!$acc parallel";
  OpenACCDirective *d2 = parseOpenACC(fpragma);
  if (d2 == nullptr) {
    std::cerr << "parseOpenACC returned NULL for Fortran pragma\n";
    return 4;
  }
  if (d2->getBaseLang() != ACC_Lang_Fortran) {
    std::cerr << "Expected ACC_Lang_Fortran but got " << d2->getBaseLang() << "\n";
    return 5;
  }

  delete d1;
  delete d2;
  std::cout << "lang_flag_test: OK\n";
  return 0;
}
