#pragma once

namespace nuis::HEPData {
struct Extent;
struct Value;
struct Variable;
struct DependentVariable;
struct Table;
} // namespace nuis::HEPData

namespace YAML {

class Node;

template <typename T> struct convert;

template <> struct convert<nuis::HEPData::Extent> {
  static bool decode(const Node &node, nuis::HEPData::Extent &ext);
};

template <> struct convert<nuis::HEPData::Value> {
  static bool decode(const Node &node, nuis::HEPData::Value &val);
};

template <> struct convert<nuis::HEPData::Variable> {
  static bool decode(const Node &node, nuis::HEPData::Variable &var);
};

template <> struct convert<nuis::HEPData::DependentVariable> {
  static bool decode(const Node &node, nuis::HEPData::DependentVariable &var);
};

template <> struct convert<nuis::HEPData::Table> {
  static bool decode(const Node &node, nuis::HEPData::Table &tbl);
};

} // namespace YAML