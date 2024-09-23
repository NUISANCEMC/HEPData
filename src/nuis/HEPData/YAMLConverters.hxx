#pragma once

#include "HEPDataRecord.hxx"

#include "yaml-cpp/yaml.h"

namespace YAML {

// HEPDataVariable
// HEPDataDependentVariable

template <> struct convert<nuis::HEPDataExtent> {
  static bool decode(const Node &node, nuis::HEPDataExtent &ext) {
    if (!node.IsMap() || !node["low"] || !node["high"]) {
      return false;
    }

    ext.low = node["low"].as<double>();
    ext.high = node["high"].as<double>();

    return true;
  }
};

template <> struct convert<nuis::HEPDataValue> {
  static bool decode(const Node &node, nuis::HEPDataValue &val) {
    if (!node.IsMap()) {
      return false;
    }

    if (node["value"]) {
      val.value = node["value"].as<double>();
    } else if (node["high"] && node["low"]) {
      val.value = node.as<nuis::HEPDataExtent>();
    } else {
      return false;
    }

    if (node["errors"]) {
      for (auto const &err : node["errors"]) {
        val.errors[err["label"].as<std::string>()] =
            err["symerror"].as<double>();
      }
    }
    return true;
  }
};

template <> struct convert<nuis::HEPDataVariable> {
  static bool decode(const Node &node, nuis::HEPDataVariable &var) {
    if (!node.IsMap() || !node["values"] || !node["header"]["name"]) {
      return false;
    }

    var.name = node["header"]["name"].as<std::string>();
    if (node["header"]["units"]) {
      var.units = node["header"]["units"].as<std::string>();
    }

    for (auto const &val : node["values"]) {
      var.values.push_back(val.as<nuis::HEPDataValue>());
    }

    return true;
  }
};

template <> struct convert<nuis::HEPDataDependentVariable> {
  static bool decode(const Node &node, nuis::HEPDataDependentVariable &var) {
    if (!node.IsMap() || !node["qualifiers"]) {
      return false;
    }

    auto const &ivar = node.as<nuis::HEPDataVariable>();

    var.name = ivar.name;
    var.units = ivar.units;
    var.values = ivar.values;

    for (auto const &qual : node["qualifiers"]) {
      var.qualifiers[qual["name"].as<std::string>()] =
          qual["value"].as<std::string>();
    }

    return true;
  }
};

template <> struct convert<nuis::HEPDataTable> {
  static bool decode(const Node &node, nuis::HEPDataTable &tbl) {
    if (!node.IsMap() || !node["dependent_variables"] ||
        !node["independent_variables"]) {
      return false;
    }

    for (auto const &ivar : node["independent_variables"]) {
      tbl.independent_vars.push_back(ivar.as<nuis::HEPDataVariable>());
    }

    for (auto const &dvar : node["dependent_variables"]) {
      tbl.dependent_vars.push_back(dvar.as<nuis::HEPDataDependentVariable>());
    }

    return true;
  }
};

} // namespace YAML