#pragma once

#include "nuis/HEPData/Variables.hxx"

#include <filesystem>

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

struct CrossSectionMeasurement : public Table {

  std::string name;
  bool is_composite;

  template <typename T> struct Weighted {
    T obj;
    double weight;
    T const &operator*() const { return obj; }
  };

  std::string variable_type;

  std::vector<std::vector<Weighted<ProbeFlux>>> probe_fluxes;
  std::vector<std::vector<Weighted<std::string>>> targets;
  std::vector<ErrorTable> errors;
  std::vector<SmearingTable> smearings;
  std::vector<CrossSectionMeasurement> sub_measurements;

  struct funcref {
    std::filesystem::path source;
    std::string fname;
  };

  std::vector<funcref> selectfuncs;
  // The outer vector corresponds to the independant variables, so all
  // projection functions for the 'y' variable live in projectfuncs[1];
  std::vector<std::vector<funcref>> projectfuncs;

  // these functions will throw if the measurement is not a simple measurement
  // with one entry for the corresponding component
  ProbeFlux const &get_single_probe_flux() const;
  std::string const &get_single_target() const;
  funcref const &get_single_selectfunc() const;
  std::vector<funcref> get_single_projectfuncs() const;
};

} // namespace nuis::HEPData