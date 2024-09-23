#pragma once

#include "nuis/HEPDataVariables.hxx"

#include <filesystem>

namespace nuis {
struct HEPDataTable {
  std::vector<HEPDataVariable> independent_vars;
  std::vector<HEPDataDependentVariable> dependent_vars;
};

struct HEPDataProbeFlux : public HEPDataTable {
  std::string probe_particle;
  std::string bin_content_type;
};

struct HEPDataErrorTable : public HEPDataTable {
  std::string error_type;
};

struct HEPDataSmearingTable : public HEPDataTable {};

struct HEPDataCrossSectionMeasurement : public HEPDataTable {

  template <typename T> struct Weighted {
    T obj;
    double weight;
    T const &operator*() const { return obj; }
  };

  std::vector<std::vector<Weighted<HEPDataProbeFlux>>> probe_fluxes;
  std::vector<std::vector<Weighted<std::string>>> targets;
  std::vector<HEPDataErrorTable> errors;
  std::vector<HEPDataSmearingTable> smearings;

  struct funcref {
    std::filesystem::path source;
    std::string fname;
  };

  std::vector<funcref> selectfuncs;
  std::vector<std::vector<funcref>> projectfuncs;
};

} // namespace nuis