#pragma once

#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>

namespace nuis::HEPData {
struct ResourceReference {

  std::string reftype;
  size_t recordid;
  int recordvers;

  // type=path references use this instead of recordid and recordvers
  std::filesystem::path path;

  std::string resourcename;
  std::string qualifier;

  std::string refstr;
  std::string context_refstr;
  bool valid;

  ResourceReference()
      : reftype{""}, recordid{0}, recordvers{0}, path{}, resourcename{""},
        qualifier{""}, refstr{""}, valid{true} {}

  ResourceReference &operator=(const ResourceReference &other) = default;

  // ref format: [<type=hepdata>:][<id>][[/]<resource[:<qualifier>]>]
  ResourceReference(std::string const &ref,
                    ResourceReference const &context = ResourceReference());

  std::optional<std::tuple<size_t, int>> parse_idv(std::string idv);
  void parse_typeidv(std::string typeidv);
  void parse_resourcequal(std::string resourceref);

  ResourceReference record_ref() const;

  std::string str() const;
  std::string component(std::string const &comp) const;
};

// ref format: /path/to/file.yaml[:<resource[:qualifier]>]
ResourceReference PathResourceReference(std::string const &refstr);

} // namespace nuis::HEPData