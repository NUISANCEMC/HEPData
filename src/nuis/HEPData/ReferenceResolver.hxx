#pragma once

#include "nuis/HEPData/ResourceReference.hxx"

#include <filesystem>

namespace nuis {

ResourceReference resolve_version(ResourceReference ref);

std::filesystem::path
resolve_reference(ResourceReference const &ref,
                  std::filesystem::path const &local_cache_root = ".");

} // namespace nuis