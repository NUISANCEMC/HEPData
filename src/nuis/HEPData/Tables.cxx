#include "nuis/HEPData/Tables.hxx"

#include "fmt/core.h"

namespace nuis::HEPData {

ProbeFlux const &CrossSectionMeasurement::get_single_probe_flux() const {
  if (probe_fluxes.size() > 1) {
    throw std::runtime_error(
        fmt::format("Called CrossSectionMeasurement::get_single_probe_flux "
                    "on a non-simple "
                    "measurement with {} probe_flux specifications.",
                    probe_fluxes.size()));
  }
  if (probe_fluxes[0].size() > 1) {
    throw std::runtime_error(
        fmt::format("Called CrossSectionMeasurement::get_single_probe_flux "
                    "on a non-simple "
                    "measurement with a probe_flux specification with {} "
                    "sub-components.",
                    probe_fluxes[0].size()));
  }

  return *probe_fluxes[0][0];
}

std::string const &CrossSectionMeasurement::get_single_target() const {
  if (targets.size() > 1) {
    throw std::runtime_error(fmt::format(
        "Called CrossSectionMeasurement::get_single_target on a non-simple "
        "measurement with {} target specifications.",
        targets.size()));
  }
  if (targets[0].size() > 1) {
    throw std::runtime_error(fmt::format(
        "Called CrossSectionMeasurement::get_single_target on a non-simple "
        "measurement with a target specification with {} "
        "sub-components.",
        targets[0].size()));
  }

  return *targets[0][0];
}

CrossSectionMeasurement::funcref const &
CrossSectionMeasurement::get_single_selectfunc() const {
  if (selectfuncs.size() > 1) {
    throw std::runtime_error(
        fmt::format("Called CrossSectionMeasurement::get_single_selectfunc "
                    "on a non-simple "
                    "measurement with {} selectfuncs.",
                    selectfuncs.size()));
  }

  return selectfuncs[0];
}

std::vector<CrossSectionMeasurement::funcref>
CrossSectionMeasurement::get_single_projectfuncs() const {
  std::vector<funcref> single_projectfuncs;

  for (size_t pfi = 0; pfi < projectfuncs.size(); ++pfi) {
    if (projectfuncs[pfi].size() != 1) {
      throw std::runtime_error(fmt::format(
          "Called CrossSectionMeasurement::get_single_projectfuncs "
          "on a non-simple "
          "measurement with {} groups of projectfuncs for independent "
          "variable: {}.",
          projectfuncs[pfi].size(), independent_vars[pfi].name));
    }
    single_projectfuncs.push_back(projectfuncs[pfi].front());
  }

  return single_projectfuncs;
}

} // namespace nuis::HEPData