#pragma once

#include "nuis/HEPData/ResourceReference.h"

#include <filesystem>

namespace nuis::HEPData {

ResourceReference resolve_version(ResourceReference ref);

std::filesystem::path
resolve_reference(ResourceReference const &ref,
                  std::filesystem::path const &local_cache_root = ".");

} // namespace nuis::HEPData