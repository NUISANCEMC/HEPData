#pragma once

#include "nuis/HEPDataTables.hxx"

#include "nuis/ResourceReference.hxx"

#include <filesystem>
#include <iostream>
#include <vector>

namespace nuis {
struct HEPDataRecord {
  std::filesystem::path record_root;
  ResourceReference record_ref;

  std::vector<HEPDataCrossSectionMeasurement> measurements;

};
} // namespace nuis