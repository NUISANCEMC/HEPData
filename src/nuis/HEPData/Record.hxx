#pragma once

#include "nuis/HEPData/Tables.hxx"

#include "nuis/HEPData/ResourceReference.hxx"

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