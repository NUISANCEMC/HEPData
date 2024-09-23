#include "nuis/HEPDataTableFactory.hxx"
#include "nuis/StreamHelpers.hxx"

#include <iostream>

int main(int argc, char const *argv[]) {
  nuis::ResourceReference a(argv[1], nuis::HEPDataRef);

  if (!a.resourcename.size()) {
    std::cout << nuis::make_HEPDataRecord(a, argc >= 2 ? argv[2] : ".") << std::endl;
  } else {
    std::cout << nuis::make_HEPDataCrossSectionMeasurement(a,
                                                      argc >= 2 ? argv[2] : ".")
              << std::endl;
  }
}