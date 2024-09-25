#pragma once

#include "nuis/HEPData/Variables.h"

#include <filesystem>
#include <string>
#include <vector>

namespace nuis::HEPData {
struct Table {
  std::filesystem::path source;
  std::vector<Variable> independent_vars;
  std::vector<DependentVariable> dependent_vars;
};

struct ProbeFlux : public Table {
  std::string probe_particle;
  std::string bin_content_type;
};

struct ErrorTable : public Table {
  std::string error_type;
};

struct SmearingTable : public Table {};

} // namespace nuis::HEPData