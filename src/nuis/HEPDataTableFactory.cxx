#include "nuis/HEPDataTables.hxx"

#include "nuis/ReferenceResolver.hxx"

#include "nuis/YAMLConverters.hxx"

#include "fmt/core.h"
#include "fmt/ranges.h"

namespace nuis {

HEPDataProbeFlux
make_HEPDataProbeFlux(ResourceReference const &ref,
                      std::filesystem::path const &local_cache_root) {
  auto tbl = YAML::LoadFile(resolve_reference(ref, local_cache_root))
                 .as<HEPDataTable>();

  HEPDataProbeFlux obj;

  for (auto const &dv : tbl.dependent_vars) {

    if (!dv.qualifiers.count("variable_type") ||
        (dv.qualifiers.at("variable_type") != "probe_flux")) {
      continue;
    }

    if (ref.qualifier.size()) {
      if (dv.name == ref.qualifier) {
        obj.independent_vars = tbl.independent_vars;
        obj.dependent_vars.push_back(dv);
        break;
      }
    } else {
      obj.independent_vars = tbl.independent_vars;
      obj.dependent_vars.push_back(dv);
      break;
    }
  }

  if (!obj.dependent_vars.size()) {
    throw std::runtime_error(fmt::format(
        "When parsing HEPDataProbeFlux from ref: \"{}\" failed to find a "
        "valid dependent variable.",
        ref.str()));
  }

  obj.probe_particle = obj.dependent_vars[0].qualifiers["probe_particle"];
  obj.bin_content_type = obj.dependent_vars[0].qualifiers["bin_content_type"];
  return obj;
}

HEPDataErrorTable
make_HEPDataErrorTable(ResourceReference const &ref,
                       std::filesystem::path const &local_cache_root) {
  auto tbl = YAML::LoadFile(resolve_reference(ref, local_cache_root))
                 .as<HEPDataTable>();

  HEPDataErrorTable obj;

  for (auto const &dv : tbl.dependent_vars) {

    if (!dv.qualifiers.count("variable_type") ||
        (dv.qualifiers.at("variable_type") != "error_table")) {
      continue;
    }

    if (ref.qualifier.size()) {
      if (dv.name == ref.qualifier) {
        obj.independent_vars = tbl.independent_vars;
        obj.dependent_vars.push_back(dv);
        break;
      }
    } else {
      obj.independent_vars = tbl.independent_vars;
      obj.dependent_vars.push_back(dv);
      break;
    }
  }

  if (!obj.dependent_vars.size()) {
    throw std::runtime_error(fmt::format(
        "When parsing HEPDataErrorTable from ref: \"{}\" failed to find a "
        "valid dependent variable.",
        ref.str()));
  }

  obj.error_type = obj.dependent_vars[0].qualifiers["error_type"];

  static std::set<std::string> const valid_error_types = {
      "covariance", "inverse_covariance", "fractional_covariance",
      "correlation", "universes"};

  if (!valid_error_types.count(obj.error_type)) {
    throw std::runtime_error(
        fmt::format("Invalid error_type qualifier: {}, must be one of {}",
                    obj.error_type, valid_error_types));
  }

  return obj;
}

std::vector<std::string> split_spec(std::string specstring) {
  std::vector<std::string> splits;

  auto comma_pos = specstring.find_first_of(',');

  while (comma_pos != std::string::npos) {
    splits.push_back(specstring.substr(0, comma_pos));
    specstring = specstring.substr(comma_pos + 1);
    comma_pos = specstring.find_first_of(',');
  }

  if (specstring.size()) {
    splits.push_back(specstring);
  }

  return splits;
}

std::pair<std::string, double> parse_weight_specifier(std::string specstring) {

  auto open_bracket_pos = specstring.find_first_of('[');

  double weight = 1;

  if (open_bracket_pos != std::string::npos) {
    weight = std::stod(
        specstring.substr(open_bracket_pos + 1, specstring.find_first_of(']') -
                                                    (open_bracket_pos + 1)));
    specstring = specstring.substr(0, open_bracket_pos);
  }

  return {specstring, weight};
}

std::vector<HEPDataCrossSectionMeasurement::Weighted<HEPDataProbeFlux>>
parse_probe_fluxes(std::string fluxsstr, ResourceReference const &ref,
                   std::filesystem::path const &local_cache_root) {

  std::vector<HEPDataCrossSectionMeasurement::Weighted<HEPDataProbeFlux>>
      flux_specs;

  for (auto const &spec : split_spec(fluxsstr)) {
    auto const &[fluxstr, weight] = parse_weight_specifier(spec);
    flux_specs.emplace_back(
        HEPDataCrossSectionMeasurement::Weighted<HEPDataProbeFlux>{
            make_HEPDataProbeFlux(ResourceReference(fluxstr, ref),
                                  local_cache_root),
            weight});
  }
  return flux_specs;
}

HEPDataCrossSectionMeasurement::funcref
make_funcref(ResourceReference const &ref,
             std::filesystem::path const &local_cache_root) {

  HEPDataCrossSectionMeasurement::funcref fref{
      resolve_reference(ref, local_cache_root), ref.qualifier};

  if (!fref.fname.size()) {
    throw std::runtime_error(
        fmt::format("Failed to resolve ref: {} to a function as it "
                    "has no qualifier to use as the function name.",
                    ref.str()));
  }

  return fref;
}

std::vector<HEPDataCrossSectionMeasurement::Weighted<std::string>>
parse_targets(std::string targetsstr) {

  std::vector<HEPDataCrossSectionMeasurement::Weighted<std::string>>
      target_specs;
  for (auto const &spec : split_spec(targetsstr)) {

    auto const &[tgtstr, weight] = parse_weight_specifier(spec);
    target_specs.emplace_back(
        HEPDataCrossSectionMeasurement::Weighted<std::string>{tgtstr, weight});
  }
  return target_specs;
}

std::optional<std::string>
get_indexed_qualifier(std::string const &key, size_t idx,
                      std::map<std::string, std::string> const &qualifiers) {

  auto key_idx = fmt::format("{}[{}]", key, idx);
  if (qualifiers.count(key_idx)) {
    return qualifiers.at(key_idx);
  } else if ((idx == 0) && qualifiers.count(key)) {
    return qualifiers.at(key);
  }
  return std::nullopt;
}

std::vector<std::string> get_indexed_qualifier_values(
    std::string const &key,
    std::map<std::string, std::string> const &qualifiers,
    bool required = false) {

  std::vector<std::string> values;

  size_t i = 0;
  while (true) { // read select funcs until you find no more
    auto iq = get_indexed_qualifier(key, i++, qualifiers);
    if (!iq) {
      if (required && (i == 0)) {
        throw std::runtime_error(fmt::format(
            "No \"{}\" found in qualifier map: {}", key, qualifiers));
      }
      break;
    }
    values.push_back(iq.value());
  }

  return values;
}

HEPDataCrossSectionMeasurement make_HEPDataCrossSectionMeasurement(
    ResourceReference const &ref,
    std::filesystem::path const &local_cache_root) {
  auto tbl = YAML::LoadFile(resolve_reference(ref, local_cache_root))
                 .as<HEPDataTable>();

  HEPDataCrossSectionMeasurement obj;

  for (auto const &dv : tbl.dependent_vars) {

    if (!dv.qualifiers.count("variable_type") ||
        (dv.qualifiers.at("variable_type") != "cross_section_measurement")) {
      continue;
    }

    if (ref.qualifier.size()) {
      if (dv.name == ref.qualifier) {
        obj.independent_vars = tbl.independent_vars;
        obj.dependent_vars.push_back(dv);
        break;
      }
    } else {
      obj.independent_vars = tbl.independent_vars;
      obj.dependent_vars.push_back(dv);
      break;
    }
  }

  if (!obj.dependent_vars.size()) {
    throw std::runtime_error(
        fmt::format("When parsing HEPDataCrossSectionMeasurement from ref: "
                    "\"{}\" failed to find a "
                    "valid dependent variable.",
                    ref.str()));
  }

  auto const &quals = obj.dependent_vars[0].qualifiers;

  for (auto const &sfuncref :
       get_indexed_qualifier_values("selectfunc", quals, true)) {
    obj.selectfuncs.emplace_back(
        make_funcref(ResourceReference(sfuncref, ref), local_cache_root));
  }

  for (auto const &tgts_spec :
       get_indexed_qualifier_values("target", quals, true)) {
    obj.targets.push_back(parse_targets(tgts_spec));
  }

  for (size_t ivi = 0; ivi < obj.independent_vars.size(); ++ivi) {

    auto const &ivar_pfuncrefs = get_indexed_qualifier_values(
        fmt::format("{}:projectfunc", obj.independent_vars[ivi].name), quals,
        true);

    if (obj.projectfuncs.size() < ivar_pfuncrefs.size()) {
      obj.projectfuncs.resize(ivar_pfuncrefs.size());
    }

    for (size_t pfi = 0; pfi < ivar_pfuncrefs.size(); ++pfi) {

      obj.projectfuncs[pfi].emplace_back(make_funcref(
          ResourceReference(ivar_pfuncrefs[pfi], ref), local_cache_root));
    }
  }

  for (auto const &probe_flux_spec :
       get_indexed_qualifier_values("probe_flux", quals, true)) {
    obj.probe_fluxes.push_back(
        parse_probe_fluxes(probe_flux_spec, ref, local_cache_root));
  }

  for (auto const &errors_spec :
       get_indexed_qualifier_values("errors", quals)) {
    obj.errors.emplace_back(make_HEPDataErrorTable(
        ResourceReference(errors_spec, ref), local_cache_root));
  }

  return obj;
}

HEPDataRecord
make_HEPDataRecord(ResourceReference const &ref,
                   std::filesystem::path const &local_cache_root) {
  HEPDataRecord obj;

  obj.record_ref = ref.record_ref();
  auto submission = resolve_reference(obj.record_ref, local_cache_root);
  obj.record_root = submission.parent_path();

  auto docs = YAML::LoadAllFromFile(submission.native());

  for (auto const &doc : docs) {
    if (doc["data_file"]) {
      auto tbl =
          YAML::LoadFile(obj.record_root / doc["data_file"].as<std::string>())
              .as<HEPDataTable>();

      for (auto const &dv : tbl.dependent_vars) {

        if (!dv.qualifiers.count("variable_type") ||
            (dv.qualifiers.at("variable_type") !=
             "cross_section_measurement")) {
          continue;
        }

        obj.measurements.emplace_back(make_HEPDataCrossSectionMeasurement(
            ResourceReference(obj.record_ref.str() + "/" +
                              doc["data_file"].as<std::string>() + ":" +
                              dv.name),
            local_cache_root));
      }
    }
  }
  return obj;
}

} // namespace nuis