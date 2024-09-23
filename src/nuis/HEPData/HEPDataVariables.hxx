#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace nuis {

struct HEPDataExtent {
  double low, high;
};

struct HEPDataValue {
  std::variant<HEPDataExtent, double> value;
  std::map<std::string, double> errors;
};

struct HEPDataVariable {
  std::vector<HEPDataValue> values;

  std::string name;
  std::string units;
};

struct HEPDataDependentVariable : public HEPDataVariable {

  std::map<std::string, std::string> qualifiers;
};

} // namespace nuis