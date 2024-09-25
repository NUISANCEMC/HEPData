#pragma once

#include "nuis/HEPData/CrossSectionMeasurement.h"
#include "nuis/HEPData/ResourceReference.h"

#include <filesystem>
#include <iostream>
#include <vector>

namespace nuis::HEPData {
struct Record {
  std::filesystem::path record_root;
  ResourceReference record_ref;

  std::vector<CrossSectionMeasurement> measurements;
  std::vector<std::filesystem::path> additional_resources;
};
} // namespace nuis::HEPData