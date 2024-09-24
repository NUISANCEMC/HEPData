#include "nuis/HEPData/ReferenceResolver.hxx"
#include "nuis/HEPData/TableFactory.hxx"
#include "nuis/HEPData/YAMLConverters.hxx"

#include "docopt.h"

#include "fmt/core.h"

#include "spdlog/spdlog.h"

#include "yaml-cpp/yaml.h"

#include <iostream>

static const char USAGE[] =
    R"(nuis-hepdata

    Usage:
      nuis-hepdata [options] get-ref-component <ref> <comp>
      nuis-hepdata [options] get-local-path <ref>
      nuis-hepdata [options] get-cross-section-measurements <ref>
      nuis-hepdata [options] get-independent-vars <ref>
      nuis-hepdata [options] get-dependent-vars <ref>
      nuis-hepdata [options] get-qualifiers <ref> [<key>]
      nuis-hepdata [options] dereference-to-local-path <ref> <key>
      nuis-hepdata [options] get-local-additional-resources <ref>
      nuis-hepdata help

    Options:
      --nuisancedb=<path>   Use <path> as the record database root.
      --debug               Enable logging for any http requests.
    

    <ref> arguments are of the form: [type:]<id>[/resource[:qualifier]]
    The <comp> argument can be one of type, id, resource, or qualifier
    <key> arguments correspond to a specific HEPData qualifier key to reference
)";

std::vector<std::string> split_spec(std::string specstring) {
  std::vector<std::string> splits;

  auto comma_pos = specstring.find_first_of(',');

  while (comma_pos != std::string::npos) {
    splits.push_back(specstring.substr(0, comma_pos));
    specstring = specstring.substr(comma_pos + 1);
    comma_pos = specstring.find_first_of(',');
  }

  if (specstring.size()) {
    splits.push_back(specstring);
  }

  return splits;
}

using namespace nuis::HEPData;

int main(int argc, const char **argv) {
  std::map<std::string, docopt::value> args =
      docopt::docopt(USAGE, {argv + 1, argv + argc},
                     true,                // show help if requested
                     "nuis-hepdata 0.9"); // version string

  std::filesystem::path local_cache_root = ".";
  if (args["--nuisancedb"]) {
    local_cache_root = args["--nuisancedb"].asString();
  } else {
    auto nuisancedb = std::getenv("NUISANCEDB");
    if (nuisancedb) {
      local_cache_root = nuisancedb;
    }
  }

  if (args["--debug"].asBool()) {
    spdlog::set_level(spdlog::level::debug);
  }

  if (!std::filesystem::exists(local_cache_root)) {
    throw std::runtime_error(
        fmt::format("record database root directory: {}, does not exist.",
                    local_cache_root.native()));
  }

  if (args["get-ref-component"].asBool()) {
    std::cout << ResourceReference(args["<ref>"].asString())
                     .component(args["<comp>"].asString())
              << std::endl;
    return 0;
  }

  if (args["get-cross-section-measurements"].asBool()) {
    for (auto const &measurement :
         make_Record(ResourceReference(args["<ref>"].asString()),
                     local_cache_root)
             .measurements) {
      std::cout << measurement.source.stem().native() << std::endl;
    }
    return 0;
  }

  if (args["get-local-path"].asBool()) {
    std::cout << resolve_reference(ResourceReference(args["<ref>"].asString()),
                                   local_cache_root)
                     .native()
              << std::endl;
    return 0;
  }

  if (args["get-independent-vars"].asBool()) {
    auto tbl = YAML::LoadFile(resolve_reference(
                                  ResourceReference(args["<ref>"].asString()),
                                  local_cache_root))
                   .as<Table>();
    for (auto const &ivar : tbl.independent_vars) {
      std::cout << ivar.name << std::endl;
    }
    return 0;
  }

  if (args["get-dependent-vars"].asBool()) {
    auto tbl = YAML::LoadFile(resolve_reference(
                                  ResourceReference(args["<ref>"].asString()),
                                  local_cache_root))
                   .as<Table>();
    for (auto const &dvar : tbl.dependent_vars) {
      std::cout << dvar.name << std::endl;
    }
    return 0;
  }

  if (args["get-qualifiers"].asBool() ||
      args["dereference-to-local-path"].asBool()) {
    auto ref = ResourceReference(args["<ref>"].asString());
    auto tbl =
        YAML::LoadFile(resolve_reference(ref, local_cache_root)).as<Table>();

    decltype(tbl.dependent_vars.front().qualifiers) quals;
    for (auto const &dvar : tbl.dependent_vars) {
      if (ref.qualifier.size()) {
        if (ref.qualifier == dvar.name) {
          quals = dvar.qualifiers;
        }
      } else {
        quals = dvar.qualifiers;
      }
    }

    bool found = false;
    for (auto const &kvp : quals) {
      if (args["<key>"]) {
        if (args["<key>"].asString() == kvp.first) {
          if (args["dereference-to-local-path"].asBool()) {
            for (auto const &el : split_spec(kvp.second)) {
              std::cout << resolve_reference(ResourceReference(el, ref),
                                             local_cache_root)
                               .native()
                        << std::endl;
            }
            found = true;
          } else {
            std::cout << kvp.second << std::endl;
            found = true;
          }
          break;
        }
      } else {
        std::cout << kvp.first << ": " << kvp.second << std::endl;
        found = true;
      }
    }

    if (!found) {
      throw std::runtime_error(
          fmt::format("failed to find specified qualifier with key: {}",
                      args["<key>"].asString()));
    }

    return 0;
  }

  if (args["get-local-additional-resources"].asBool()) {
    for (auto const &addres :
         make_Record(ResourceReference(args["<ref>"].asString()),
                     local_cache_root)
             .additional_resources) {
      std::cout << addres.filename().native() << std::endl;
    }
  }

  if (args["help"].asBool()) {
    std::cout << USAGE << std::endl;
    return 0;
  }

  return 0;
}