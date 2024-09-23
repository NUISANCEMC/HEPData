#pragma once

#include "nuis/HEPData/HEPDataRecord.hxx"

#include "nuis/HEPData/ResourceReference.hxx"

#include <filesystem>

namespace nuis {
HEPDataProbeFlux
make_HEPDataProbeFlux(ResourceReference ref,
                      std::filesystem::path const &local_cache_root = ".");
HEPDataErrorTable
make_HEPDataErrorTable(ResourceReference ref,
                       std::filesystem::path const &local_cache_root = ".");
HEPDataCrossSectionMeasurement make_HEPDataCrossSectionMeasurement(
    ResourceReference ref, std::filesystem::path const &local_cache_root = ".");

HEPDataRecord
make_HEPDataRecord(ResourceReference ref,
                   std::filesystem::path const &local_cache_root = ".");
} // namespace nuis