#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <tuple>

namespace nuis {
struct ResourceReference {

  std::string reftype;
  size_t recordid;
  int recordvers;
  std::string resourcename;
  std::string qualifier;

  std::string refstr;
  std::string context_refstr;
  bool valid;

  ResourceReference()
      : reftype{""}, recordid{0}, recordvers{0}, resourcename{""},
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
};

extern ResourceReference const HEPDataRef;

} // namespace nuis