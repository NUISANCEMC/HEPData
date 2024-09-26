#include "nuis/HEPData/TableFactory.h"
#include "nuis/HEPData/CrossSectionMeasurement.h"
#include "nuis/HEPData/ReferenceResolver.h"
#include "nuis/HEPData/YAMLConverters.h"

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "yaml-cpp/yaml.h"

#include "spdlog/spdlog.h"

namespace nuis::HEPData {

ProbeFlux make_ProbeFlux(ResourceReference ref,
                         std::filesystem::path const &local_cache_root) {

  auto source = resolve_reference(ref, local_cache_root);
  auto tbl = YAML::LoadFile(source).as<Table>();

  ProbeFlux obj;
  obj.source = source;

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
    throw std::runtime_error(
        fmt::format("When parsing ProbeFlux from ref: \"{}\" failed to find a "
                    "valid dependent variable.",
                    ref.str()));
  }

  obj.probe_particle = obj.dependent_vars[0].qualifiers["probe_particle"];
  obj.bin_content_type = obj.dependent_vars[0].qualifiers["bin_content_type"];
  return obj;
}

ErrorTable make_ErrorTable(ResourceReference ref,
                           std::filesystem::path const &local_cache_root) {

  auto source = resolve_reference(ref, local_cache_root);
  auto tbl = YAML::LoadFile(source).as<Table>();

  ErrorTable obj;
  obj.source = source;

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
    throw std::runtime_error(
        fmt::format("When parsing ErrorTable from ref: \"{}\" failed to find a "
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

std::vector<std::string> split_spec(std::string specstring, char delim = ',') {
  std::vector<std::string> splits;

  auto comma_pos = specstring.find_first_of(delim);

  while (comma_pos != std::string::npos) {
    splits.push_back(specstring.substr(0, comma_pos));
    specstring = specstring.substr(comma_pos + 1);
    comma_pos = specstring.find_first_of(delim);
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

std::vector<CrossSectionMeasurement::Weighted<ProbeFlux>>
parse_probe_fluxes(std::string fluxsstr, ResourceReference ref,
                   std::filesystem::path const &local_cache_root) {

  std::vector<CrossSectionMeasurement::Weighted<ProbeFlux>> flux_specs;

  for (auto const &spec : split_spec(fluxsstr)) {
    auto const &[fluxstr, weight] = parse_weight_specifier(spec);
    flux_specs.emplace_back(CrossSectionMeasurement::Weighted<ProbeFlux>{
        make_ProbeFlux(ResourceReference(fluxstr, ref), local_cache_root),
        weight});
  }
  return flux_specs;
}

CrossSectionMeasurement::funcref
make_funcref(ResourceReference ref,
             std::filesystem::path const &local_cache_root) {

  CrossSectionMeasurement::funcref fref{
      resolve_reference(ref, local_cache_root), ref.qualifier};

  if (!fref.fname.size()) {
    throw std::runtime_error(
        fmt::format("Failed to resolve ref: {} to a function as it "
                    "has no qualifier to use as the function name.",
                    ref.str()));
  }

  return fref;
}

inline int NuclearPDGToA(int pid) {
  // ±10LZZZAAAI
  return (pid / 1) % 1000;
}

inline int NuclearPDGToZ(int pid) {
  // ±10LZZZAAAI
  return (pid / 10000) % 1000;
}

CrossSectionMeasurement::TargetList
tgtstr_to_target(std::string const &tgtstr) {
  using wgtgt = CrossSectionMeasurement::TargetList::value_type;
  try {
    auto pid = std::stol(tgtstr);
    return {
        wgtgt{CrossSectionMeasurement::Target{NuclearPDGToA(pid),
                                              NuclearPDGToZ(pid)},
              1},
    };
  } catch (std::invalid_argument &ia) {
  }

  if (tgtstr == "C") {
    return {wgtgt{CrossSectionMeasurement::Target{12, 6}, 1}};
  } else if (tgtstr == "CH") {
    return {wgtgt{CrossSectionMeasurement::Target{12, 6}, 1},
            wgtgt{CrossSectionMeasurement::Target{1, 1}, 1}};
  } else if (tgtstr == "CH2") {
    return {wgtgt{CrossSectionMeasurement::Target{12, 6}, 1},
            wgtgt{CrossSectionMeasurement::Target{1, 1}, 2}};
  } else if (tgtstr == "O") {
    return {wgtgt{CrossSectionMeasurement::Target{16, 8}, 1}};
  } else if (tgtstr == "H2O") {
    return {wgtgt{CrossSectionMeasurement::Target{16, 8}, 1},
            wgtgt{CrossSectionMeasurement::Target{1, 1}, 2}};
  } else if (tgtstr == "Ar") {
    return {wgtgt{CrossSectionMeasurement::Target{40, 18}, 1}};
  } else if (tgtstr == "Fe") {
    return {wgtgt{CrossSectionMeasurement::Target{56, 26}, 1}};
  } else if (tgtstr == "Pb") {
    return {wgtgt{CrossSectionMeasurement::Target{208, 82}, 1}};
  }

  throw std::runtime_error(
      fmt::format("failed to parse target string: {}", tgtstr));
}

CrossSectionMeasurement::TargetList parse_targets(std::string targetsstr) {

  CrossSectionMeasurement::TargetList target_specs;
  for (auto const &spec : split_spec(targetsstr)) {

    auto const &[tgtstr, weight] = parse_weight_specifier(spec);
    auto const &targets = tgtstr_to_target(tgtstr);

    for (auto tgt : targets) {
      // always weight by A so that weights correspond to mass ratio even when
      // unspecified
      tgt.weight *= weight * double(tgt->A);
      target_specs.emplace_back(tgt);
    }
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

  while (true) { // read select funcs until you find no more
    auto iq = get_indexed_qualifier(key, values.size(), qualifiers);
    if (!iq) {
      if (required && !values.size()) {
        throw std::runtime_error(fmt::format(
            "No \"{}\" found in qualifier map: {}", key, qualifiers));
      }
      break;
    }
    values.push_back(iq.value());
  }

  return values;
}

static std::set<std::string> const valid_variable_types = {
    "cross_section_measurement", "composite_cross_section_measurement"};

CrossSectionMeasurement
make_CrossSectionMeasurement(ResourceReference ref,
                             std::filesystem::path const &local_cache_root) {

  auto source = resolve_reference(ref, local_cache_root);
  auto tbl = YAML::LoadFile(source).as<Table>();

  CrossSectionMeasurement obj;
  obj.source = source;

  for (auto const &dv : tbl.dependent_vars) {

    if (!dv.qualifiers.count("variable_type") ||
        (!valid_variable_types.count(dv.qualifiers.at("variable_type")))) {
      continue;
    }

    if (ref.qualifier.size()) {
      if (dv.name == ref.qualifier) {
        obj.independent_vars = tbl.independent_vars;
        obj.dependent_vars.push_back(dv);
        obj.is_composite = (dv.qualifiers.at("variable_type") ==
                            "composite_cross_section_measurement");
        break;
      }
    } else {
      obj.independent_vars = tbl.independent_vars;
      obj.dependent_vars.push_back(dv);
      obj.is_composite = (dv.qualifiers.at("variable_type") ==
                          "composite_cross_section_measurement");
      break;
    }
  }

  if (!obj.dependent_vars.size()) {
    throw std::runtime_error(
        fmt::format("When parsing CrossSectionMeasurement from ref: "
                    "\"{}\" failed to find a "
                    "valid dependent variable.",
                    ref.str()));
  }

  auto const &quals = obj.dependent_vars[0].qualifiers;

  for (auto const &sfuncref :
       get_indexed_qualifier_values("selectfunc", quals, !obj.is_composite)) {
    obj.selectfuncs.emplace_back(
        make_funcref(ResourceReference(sfuncref, ref), local_cache_root));
  }

  for (auto const &tgts_spec :
       get_indexed_qualifier_values("target", quals, !obj.is_composite)) {
    obj.targets.push_back(parse_targets(tgts_spec));
  }

  for (auto const &ivar : obj.independent_vars) {

    obj.projectfuncs.emplace_back();

    for (auto const &ivpf :
         get_indexed_qualifier_values(fmt::format("{}:projectfunc", ivar.name),
                                      quals, !obj.is_composite)) {

      obj.projectfuncs.back().emplace_back(
          make_funcref(ResourceReference(ivpf, ref), local_cache_root));
    }
  }

  for (auto const &probe_flux_spec :
       get_indexed_qualifier_values("probe_flux", quals, !obj.is_composite)) {
    obj.probe_fluxes.push_back(
        parse_probe_fluxes(probe_flux_spec, ref, local_cache_root));
  }

  for (auto const &errors_spec :
       get_indexed_qualifier_values("errors", quals)) {
    obj.errors.emplace_back(
        make_ErrorTable(ResourceReference(errors_spec, ref), local_cache_root));
  }

  if (obj.is_composite && quals.count("sub_measurements")) {
    for (auto const &sub_ref : split_spec(quals.at("sub_measurements"))) {
      obj.sub_measurements.emplace_back(make_CrossSectionMeasurement(
          ResourceReference(sub_ref, ref), local_cache_root));
    }
  }

  obj.measurement_type = "flux_averaged_differential_cross_section";
  if (quals.count("measurement_type")) {
    obj.measurement_type = quals.at("measurement_type");

    static std::set<std::string> const valid_measurement_types = {
        "flux_averaged_differential_cross_section", "event_rate", "ratio",
        "total_cross_section"};
    if (!valid_measurement_types.count(obj.measurement_type)) {
      throw std::runtime_error(fmt::format(
          "invalid measurement_type qualifier found: {}, valid types: {}",
          obj.measurement_type, valid_measurement_types));
    }
  }

  if (quals.count("cross_section_units")) {
    for (auto const &u : split_spec(quals.at("cross_section_units"), '|')) {
      obj.cross_section_units.insert(u);
    }
  }

  obj.test_statistic = "chi2";
  if (quals.count("test_statistic")) {
    obj.test_statistic = quals.at("test_statistic");

    static std::set<std::string> const valid_test_statistics = {
        "chi2", "shape_only_chi2", "shape_plus_norm_chi2", "poisson_pdf"};
    if (!valid_test_statistics.count(obj.test_statistic)) {
      throw std::runtime_error(fmt::format(
          "invalid test_statistic qualifier found: {}, valid types: {}",
          obj.test_statistic, valid_test_statistics));
    }
  }

  return obj;
}

Record make_Record(ResourceReference ref,
                   std::filesystem::path const &local_cache_root) {
  Record obj;

  ref = resolve_version(ref);

  obj.record_ref = ref.record_ref();
  auto submission = resolve_reference(obj.record_ref, local_cache_root);
  obj.record_root = submission.parent_path();
  spdlog::debug("make_Record(ref={}), loading documents from file: {}",
                ref.str(), submission.native());

  auto docs = YAML::LoadAllFromFile(submission.native());

  for (auto const &doc : docs) {
    if (doc["data_file"]) {
      auto data_file_path =
          obj.record_root / doc["data_file"].as<std::string>();
      auto tbl = YAML::LoadFile(data_file_path.native()).as<Table>();

      spdlog::debug("  -> loading data_file: {}", data_file_path.native());

      for (auto const &dv : tbl.dependent_vars) {
        spdlog::debug("    -> dependent_variable(name={}, type={})", dv.name,
                      (dv.qualifiers.count("variable_type")
                           ? dv.qualifiers.at("variable_type")
                           : "n/a"));

        if (!dv.qualifiers.count("variable_type") ||
            !valid_variable_types.count(dv.qualifiers.at("variable_type"))) {
          continue;
        }

        obj.measurements.emplace_back(make_CrossSectionMeasurement(
            ResourceReference(fmt::format("{}:{}",
                                          doc["data_file"].as<std::string>(),
                                          dv.name),
                              ref),
            local_cache_root));
        obj.measurements.back().name = doc["name"].as<std::string>();
      }
    } else {
      spdlog::debug("no data_file in doc: ");
      YAML::Dump(doc);
    }

    for (auto const &addres : doc["additional_resources"]) {
      auto expected_location =
          obj.record_root / addres["location"].as<std::string>();
      if (std::filesystem::exists(expected_location)) {
        obj.additional_resources.push_back(expected_location);
      }
    }
  }
  return obj;
}

Record make_Record(std::filesystem::path const &location,
                   std::filesystem::path const &local_cache_root) {

  return make_Record(PathResourceReference(location), local_cache_root);
}

} // namespace nuis::HEPData