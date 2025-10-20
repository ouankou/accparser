#include <iostream>
#include <string>
#include "OpenACCIR.h"

extern OpenACCDirective *parseOpenACC(std::string);

int main() {
  std::string pragma = "#pragma acc parallel";
  OpenACCDirective *d = parseOpenACC(pragma);
  if (!d) {
    std::cerr << "parseOpenACC returned NULL" << std::endl;
    return 2;
  }
  if (d->getBaseLang() != ACC_Lang_C) {
    std::cerr << "unexpected base lang: " << d->getBaseLang() << std::endl;
    delete d;
    return 3;
  }
  delete d;
  std::cout << "ok" << std::endl;
  return 0;
}
