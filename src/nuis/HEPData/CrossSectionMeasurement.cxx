#include "nuis/HEPData/CrossSectionMeasurement.h"

#include "fmt/core.h"

namespace nuis::HEPData {

ProbeFlux const &CrossSectionMeasurement::get_single_probe_flux() const {
  if (probe_fluxes.size() != 1) {
    throw std::runtime_error(
        fmt::format("Called CrossSectionMeasurement::get_single_probe_flux "
                    "on a non-simple "
                    "measurement with {} probe_flux specifications.",
                    probe_fluxes.size()));
  }
  if (probe_fluxes[0].size() != 1) {
    throw std::runtime_error(
        fmt::format("Called CrossSectionMeasurement::get_single_probe_flux "
                    "on a non-simple "
                    "measurement with a probe_flux specification with {} "
                    "sub-components.",
                    probe_fluxes[0].size()));
  }

  return *probe_fluxes[0][0];
}

ErrorTable const &CrossSectionMeasurement::get_single_errors() const {
  if (errors.size() != 1) {
    throw std::runtime_error(
        fmt::format("Called CrossSectionMeasurement::get_single_errors "
                    "on a non-simple "
                    "measurement with {} error specifications.",
                    errors.size()));
  }

  return errors[0];
}

SmearingTable const &CrossSectionMeasurement::get_single_smearing() const {
  if (smearings.size() != 1) {
    throw std::runtime_error(
        fmt::format("Called CrossSectionMeasurement::get_single_smearing "
                    "on a non-simple "
                    "measurement with {} error specifications.",
                    smearings.size()));
  }

  return smearings[0];
}

std::pair<double, double> CrossSectionMeasurement::get_simple_target() const {
  if (targets.size() != 1) {
    throw std::runtime_error(fmt::format(
        "Called CrossSectionMeasurement::get_simple_target on a non-simple "
        "measurement with {} target specifications.",
        targets.size()));
  }
  std::pair<double, double> AZ{0, 0};

  double sumw = 0;

  for (auto const &tgt : targets[0]) {
    AZ.first += tgt->A * tgt.weight;
    AZ.second += tgt->Z * tgt.weight;
    sumw += tgt.weight;
  }

  AZ.first /= sumw;
  AZ.second /= sumw;

  return AZ;
}

CrossSectionMeasurement::funcref const &
CrossSectionMeasurement::get_single_selectfunc() const {
  if (selectfuncs.size() != 1) {
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

  if (single_projectfuncs.size() != independent_vars.size()) {
    throw std::runtime_error(
        fmt::format("Called CrossSectionMeasurement::get_single_projectfuncs "
                    "on a non-simple measurement, found {} projectfuncs for {} "
                    "independent_vars.",
                    single_projectfuncs.size(), independent_vars.size()));
  }

  return single_projectfuncs;
}

std::vector<std::string>
CrossSectionMeasurement::get_single_project_prettynames() const {
  std::vector<std::string> single_project_prettynames;

  for (size_t pfi = 0; pfi < project_prettynames.size(); ++pfi) {
    if (project_prettynames[pfi].size() > 1) {
      throw std::runtime_error(fmt::format(
          "Called CrossSectionMeasurement::get_single_project_prettynames "
          "on a non-simple "
          "measurement with {} groups of project_prettynames for independent "
          "variable: {}.",
          project_prettynames[pfi].size(), independent_vars[pfi].name));
    }
    if (project_prettynames[pfi].size() == 1) {
      single_project_prettynames.push_back(project_prettynames[pfi].front());
    } else {
      single_project_prettynames.push_back("");
    }
  }

  return single_project_prettynames;
}

} // namespace nuis::HEPData