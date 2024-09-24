#include "nuis/HEPData/YAMLConverters.hxx"

#include "nuis/HEPData/Record.hxx"

#include "yaml-cpp/yaml.h"

using namespace nuis;

namespace YAML {

bool convert<HEPData::Extent>::decode(const Node &node, HEPData::Extent &ext) {
  if (!node.IsMap() || !node["low"] || !node["high"]) {
    return false;
  }

  ext.low = node["low"].as<double>();
  ext.high = node["high"].as<double>();

  return true;
}

bool convert<HEPData::Value>::decode(const Node &node, HEPData::Value &val) {
  if (!node.IsMap()) {
    return false;
  }

  if (node["value"]) {
    val.value = node["value"].as<double>();
  } else if (node["high"] && node["low"]) {
    val.value = node.as<HEPData::Extent>();
  } else {
    return false;
  }

  if (node["errors"]) {
    for (auto const &err : node["errors"]) {
      val.errors[err["label"].as<std::string>()] = err["symerror"].as<double>();
    }
  }
  return true;
}

bool convert<HEPData::Variable>::decode(const Node &node,
                                        HEPData::Variable &var) {
  if (!node.IsMap() || !node["values"] || !node["header"]["name"]) {
    return false;
  }

  var.name = node["header"]["name"].as<std::string>();
  if (node["header"]["units"]) {
    var.units = node["header"]["units"].as<std::string>();
  }

  for (auto const &val : node["values"]) {
    var.values.push_back(val.as<HEPData::Value>());
  }

  return true;
}

bool convert<HEPData::DependentVariable>::decode(
    const Node &node, HEPData::DependentVariable &var) {
  if (!node.IsMap() || !node["qualifiers"]) {
    return false;
  }

  auto const &ivar = node.as<HEPData::Variable>();

  var.name = ivar.name;
  var.units = ivar.units;
  var.values = ivar.values;

  for (auto const &qual : node["qualifiers"]) {
    var.qualifiers[qual["name"].as<std::string>()] =
        qual["value"].as<std::string>();
  }

  return true;
}

bool convert<HEPData::Table>::decode(const Node &node, HEPData::Table &tbl) {
  if (!node.IsMap() || !node["dependent_variables"] ||
      !node["independent_variables"]) {
    return false;
  }

  for (auto const &ivar : node["independent_variables"]) {
    tbl.independent_vars.push_back(ivar.as<HEPData::Variable>());
  }

  for (auto const &dvar : node["dependent_variables"]) {
    tbl.dependent_vars.push_back(dvar.as<HEPData::DependentVariable>());
  }

  return true;
}

} // namespace YAML