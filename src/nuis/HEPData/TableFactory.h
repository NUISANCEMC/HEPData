#pragma once

#include "nuis/HEPData/Record.h"
#include "nuis/HEPData/ResourceReference.h"

#include <filesystem>

namespace nuis::HEPData {

ProbeFlux make_ProbeFlux(ResourceReference ref,
                         std::filesystem::path const &local_cache_root = ".");

ErrorTable make_ErrorTable(ResourceReference ref,
                           std::filesystem::path const &local_cache_root = ".");

CrossSectionMeasurement make_CrossSectionMeasurement(
    ResourceReference ref, std::filesystem::path const &local_cache_root = ".");

Record make_Record(std::filesystem::path const &location,
                   std::filesystem::path const &local_cache_root = ".");

Record make_Record(ResourceReference ref,
                   std::filesystem::path const &local_cache_root = ".");

} // namespace nuis::HEPData