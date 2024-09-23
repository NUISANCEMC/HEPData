#pragma once

#include "nuis/HEPDataRecord.hxx"

#include "nuis/ResourceReference.hxx"

#include <filesystem>

namespace nuis {
HEPDataProbeFlux
make_HEPDataProbeFlux(ResourceReference const &ref,
                      std::filesystem::path const &local_cache_root = ".");
HEPDataErrorTable
make_HEPDataErrorTable(ResourceReference const &ref,
                       std::filesystem::path const &local_cache_root = ".");
HEPDataCrossSectionMeasurement make_HEPDataCrossSectionMeasurement(
    ResourceReference const &ref,
    std::filesystem::path const &local_cache_root = ".");

HEPDataRecord
make_HEPDataRecord(ResourceReference const &ref,
                   std::filesystem::path const &local_cache_root = ".");
} // namespace nuis