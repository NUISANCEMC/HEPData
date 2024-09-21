#pragma once

#include "nuis/HEPDataResource.hxx"
#include "nuis/HEPDataTable.hxx"

#include "nuis/YAMLConverters.hxx"

#include "nuis/ReferenceResolver.hxx"
#include "nuis/ResourceReference.hxx"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <iostream>
#include <vector>

namespace nuis {
struct HEPDataRecord {
  std::filesystem::path record_root;
  ResourceReference record_ref;

  std::vector<HEPDataCrossSectionMeasurement> measurements;

  HEPDataRecord(ResourceReference const &ref,
                std::filesystem::path const &local_cache_root = ".") {
    record_ref = ref.record_ref();
    auto submission = resolve_reference(record_ref, local_cache_root);
    record_root = submission.parent_path();

    auto docs = YAML::LoadAllFromFile(submission.native());

    for (auto const &doc : docs) {
      if (doc["data_file"]) {
        auto tbl =
            YAML::LoadFile(record_root / doc["data_file"].as<std::string>())
                .as<HEPDataTable>();

        for (auto const &dv : tbl.dependent_vars) {

          if (!dv.qualifiers.count("variable_type") ||
              (dv.qualifiers.at("variable_type") !=
               "cross_section_measurement")) {
            continue;
          }

          measurements.emplace_back(
              ResourceReference(record_ref.str() + "/" +
                                doc["data_file"].as<std::string>() + ":" +
                                dv.name),
              local_cache_root);
        }
      }
    }
  }
};
} // namespace nuis