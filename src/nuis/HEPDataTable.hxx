#pragma once

#include "nuis/HEPDataVariables.hxx"

#include "nuis/ReferenceResolver.hxx"
#include "nuis/ResourceReference.hxx"

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"

namespace nuis {
struct HEPDataTable {

  std::vector<HEPDataVariable> independent_vars;
  std::vector<HEPDataDependentVariable> dependent_vars;
};

struct HEPDataProbeFlux : public HEPDataTable {

  std::string probe_particle;
  std::string bin_content_type;

  HEPDataProbeFlux(ResourceReference const &ref,
                   std::filesystem::path const &local_cache_root = ".") {
    auto tbl = YAML::LoadFile(resolve_reference(ref, local_cache_root))
                   .as<HEPDataTable>();

    for (auto const &dv : tbl.dependent_vars) {

      if (!dv.qualifiers.count("variable_type") ||
          (dv.qualifiers.at("variable_type") != "probe_flux")) {
        continue;
      }

      if (ref.qualifier.size()) {
        if (dv.name == ref.qualifier) {
          independent_vars = tbl.independent_vars;
          dependent_vars.push_back(dv);
          break;
        }
      } else {
        independent_vars = tbl.independent_vars;
        dependent_vars.push_back(dv);
        break;
      }
    }

    if (!dependent_vars.size()) {
      throw std::runtime_error(fmt::format(
          "When parsing HEPDataProbeFlux from ref: \"{}\" failed to find a "
          "valid dependent variable.",
          ref.str()));
    }

    probe_particle = dependent_vars[0].qualifiers["probe_particle"];
    bin_content_type = dependent_vars[0].qualifiers["bin_content_type"];
  }
};

struct HEPDataErrorTable : public HEPDataTable {

  std::string error_type;

  HEPDataErrorTable(ResourceReference const &ref,
                    std::filesystem::path const &local_cache_root = ".") {
    auto tbl = YAML::LoadFile(resolve_reference(ref, local_cache_root))
                   .as<HEPDataTable>();

    for (auto const &dv : tbl.dependent_vars) {

      if (!dv.qualifiers.count("variable_type") ||
          (dv.qualifiers.at("variable_type") != "error_table")) {
        continue;
      }

      if (ref.qualifier.size()) {
        if (dv.name == ref.qualifier) {
          independent_vars = tbl.independent_vars;
          dependent_vars.push_back(dv);
          break;
        }
      } else {
        independent_vars = tbl.independent_vars;
        dependent_vars.push_back(dv);
        break;
      }
    }

    if (!dependent_vars.size()) {
      throw std::runtime_error(fmt::format(
          "When parsing HEPDataErrorTable from ref: \"{}\" failed to find a "
          "valid dependent variable.",
          ref.str()));
    }

    error_type = dependent_vars[0].qualifiers["error_type"];
  }
};

struct HEPDataSmearingTable : public HEPDataTable {};

struct HEPDataCrossSectionMeasurement : public HEPDataTable {

  std::vector<HEPDataProbeFlux> probe_fluxes;
  std::vector<HEPDataErrorTable> errors;
  std::vector<HEPDataSmearingTable> smearings;

  ResourceReference selectfunc;
  std::vector<ResourceReference> projectfuncs;
  std::string target;

  HEPDataCrossSectionMeasurement(
      ResourceReference const &ref,
      std::filesystem::path const &local_cache_root = ".") {
    auto tbl = YAML::LoadFile(resolve_reference(ref, local_cache_root))
                   .as<HEPDataTable>();

    for (auto const &dv : tbl.dependent_vars) {

      if (!dv.qualifiers.count("variable_type") ||
          (dv.qualifiers.at("variable_type") != "cross_section_measurement")) {
        continue;
      }

      if (ref.qualifier.size()) {
        if (dv.name == ref.qualifier) {
          independent_vars = tbl.independent_vars;
          dependent_vars.push_back(dv);
          break;
        }
      } else {
        independent_vars = tbl.independent_vars;
        dependent_vars.push_back(dv);
        break;
      }
    }

    if (!dependent_vars.size()) {
      throw std::runtime_error(
          fmt::format("When parsing HEPDataCrossSectionMeasurement from ref: "
                      "\"{}\" failed to find a "
                      "valid dependent variable.",
                      ref.str()));
    }

    selectfunc =
        ResourceReference(dependent_vars[0].qualifiers["selectfunc"], ref);
    target = dependent_vars[0].qualifiers["target"];

    for (auto const &ivar : independent_vars) {
      projectfuncs.push_back(ResourceReference(
          dependent_vars[0]
              .qualifiers[fmt::format("{}:projectfunc", ivar.name)],
          ref));
    }

    probe_fluxes.emplace_back(
        ResourceReference(dependent_vars[0].qualifiers["probe_flux"], ref),
        local_cache_root);
  }
};

} // namespace nuis