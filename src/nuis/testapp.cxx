#include "nuis/HEPDataRecord.hxx"
#include "nuis/StreamHelpers.hxx"

#include <iostream>

int main(int argc, char const *argv[]) {
  nuis::ResourceReference a(argv[1], nuis::HEPDataRef);
  std::cout << nuis::HEPDataRecord(a, argc >= 2 ? argv[2] : ".") << std::endl;
}