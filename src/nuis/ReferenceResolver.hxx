#pragma once

#include "nuis/ResourceReference.hxx"

#include <filesystem>

namespace nuis {

std::filesystem::path
resolve_reference(ResourceReference const &ref,
                  std::filesystem::path const &local_cache_root = ".");

} // namespace nuis