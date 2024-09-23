#pragma once

#include "nuis/HEPData/HEPDataTables.hxx"

#include "nuis/HEPData/ResourceReference.hxx"

#include <filesystem>
#include <iostream>
#include <vector>

namespace nuis {
struct HEPDataRecord {
  std::filesystem::path record_root;
  ResourceReference record_ref;

  std::vector<HEPDataCrossSectionMeasurement> measurements;
  std::vector<std::filesystem::path> additional_resources;

};
} // namespace nuis