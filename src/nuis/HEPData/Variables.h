#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace nuis::HEPData {

struct Extent {
  double low, high;
};

struct Value {
  std::variant<Extent, double> value;
  std::map<std::string, double> errors;
};

struct Variable {
  std::vector<Value> values;

  std::string name;
  std::string units;
};

struct DependentVariable : public Variable {

  std::map<std::string, std::string> qualifiers;

  std::string prettyname;
};

} // namespace nuis::HEPData