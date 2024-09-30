#pragma once

#include "nuis/HEPData/Tables.h"

#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace nuis::HEPData {

struct CrossSectionMeasurement : public Table {

  std::string name;
  bool is_composite;

  template <typename T> struct Weighted {
    T obj;
    double weight;
    T const &operator*() const { return obj; }
    T const *operator->() const { return &obj; }
  };

  std::string variable_type;
  std::string measurement_type;
  std::set<std::string> cross_section_units;
  std::string test_statistic;

  struct Target {
    int A, Z;
  };

  std::vector<std::vector<Weighted<ProbeFlux>>> probe_fluxes;

  using TargetList = std::vector<Weighted<Target>>;
  std::vector<TargetList> targets;
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

  // prettynames for projections that can contain characters inappropriate for
  // an independent variable name. This includes names including latex math.
  std::vector<std::vector<std::string>> project_prettynames;
  // these functions will throw if the measurement is not a simple measurement
  // with one entry for the corresponding component
  ProbeFlux const &get_single_probe_flux() const;

  ErrorTable const &get_single_errors() const;
  SmearingTable const &get_single_smearing() const;

  // This works for measurements with a single list of targets, it sums up the
  // weighted proton and neutron numbers of all targets,
  //   e.g. a CH2 target -> A=14, Z=8.
  // This object has limited utility but can be useful for non-standard cross
  // section unit scalings.
  std::pair<double, double> get_simple_target() const;
  funcref const &get_single_selectfunc() const;
  std::vector<funcref> get_single_projectfuncs() const;
  std::vector<std::string> get_single_project_prettynames() const;
};

} // namespace nuis::HEPData