#include "nuis/HEPData/ReferenceResolver.h"

#include "cpr/cpr.h"

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"

#include "spdlog/spdlog.h"

#include <fstream>
#include <iostream>

namespace nuis::HEPData {

std::filesystem::path
get_expected_record_location(ResourceReference const &ref,
                             std::filesystem::path const &local_cache_root) {

  std::filesystem::path expected_location = local_cache_root;

  if (ref.reftype == "hepdata") {
    expected_location /= fmt::format("hepdata/{0}/HEPData-{0}-v{1}",
                                     ref.recordid, ref.recordvers);
  } else if (ref.reftype == "hepdata-sandbox") {
    expected_location /= fmt::format("hepdata-sandbox/{0}/HEPData-{0}-v{1}",
                                     ref.recordid, ref.recordvers);
  } else if (ref.reftype == "inspirehep") {
    expected_location /= fmt::format("INSPIREHEP/{0}", ref.recordid);
  }

  return expected_location;
}

std::filesystem::path
get_expected_resource_location(ResourceReference const &ref,
                               std::filesystem::path const &local_cache_root) {

  std::filesystem::path expected_location =
      get_expected_record_location(ref, local_cache_root);

  if (ref.resourcename.size()) {
    expected_location /= ref.resourcename;
  } else {
    expected_location /= "submission.yaml";
  }

  return expected_location;
}

cpr::Url get_record_endpoint(ResourceReference const &ref) {
  cpr::Url Endpoint{"https://www.hepdata.net/record/"};

  if (ref.reftype == "hepdata") {
    Endpoint += fmt::format("{}", ref.recordid);
  } else if (ref.reftype == "hepdata-sandbox") {
    Endpoint += fmt::format("sandbox/{}", ref.recordid);
  } else if (ref.reftype == "inspirehep") {
    Endpoint += fmt::format("ins{}", ref.recordid);
  }

  return Endpoint;
}

std::filesystem::path
ensure_local_path(ResourceReference const &ref,
                  std::filesystem::path const &local_cache_root) {

  auto expected_location =
      get_expected_resource_location(ref, local_cache_root);

  std::string yaml_opt = expected_location.extension().empty() ? "[.yaml]" : "";

  spdlog::debug("ensure_local_path(ref={},local_cache_root={}): "
                "expected_location = {}{}",
                ref.str(), local_cache_root.native(),
                expected_location.native(), yaml_opt);

  if (std::filesystem::exists(expected_location)) {
    return expected_location;
  }

  auto expected_location_yaml = expected_location;
  expected_location_yaml += ".yaml";
  // also check if the resource is the table name with a corresponding yaml file
  if (std::filesystem::exists(expected_location_yaml)) {
    return expected_location_yaml;
  }

  auto record_location = get_expected_record_location(ref, local_cache_root);

  // if the submission exists, then it is likely that this resource is mispelled
  if (std::filesystem::exists(record_location)) {
    std::stringstream dir_contents;
    for (auto const &dir_entry :
         std::filesystem::directory_iterator{record_location}) {
      dir_contents << "  " << dir_entry.path().native() << '\n';
    }

    throw std::runtime_error(fmt::format(
        "Failed to resolve reference: {} to a file in the submission "
        "directory: "
        "{}, with contents:\n{}\n\nIs the resourcename, {}, correct?",
        ref.str(), record_location.native(), dir_contents.str(),
        ref.resourcename));
  }

  if (ref.reftype == "inspirehep") {
    throw std::runtime_error(
        "Cannot yet fetch non-local inspirehep-type resources.");
  }

  std::filesystem::path download_location = record_location / "submission.zip";

  std::filesystem::create_directories(record_location);

  std::ofstream of(download_location, std::ios::binary);

  cpr::Url Endpoint = get_record_endpoint(ref);

  spdlog::debug("Doesn't exist, downloading...");
  spdlog::debug("  GET {} -> {} ", Endpoint.str(), download_location.native());

  cpr::Response r =
      cpr::Download(of, Endpoint, cpr::Parameters{{"format", "original"}});

  spdlog::debug("   http response --> {} ", r.status_code);

  if (r.status_code != 200) {
    throw std::runtime_error(
        fmt::format("GET response code: {}", r.status_code));
  }

  if (r.header["content-type"] != "application/zip") {
    throw std::runtime_error(fmt::format(
        "GET response content-type: {}, expected \"application/zip\"",
        r.header["content-type"]));
  }

  std::string redir = "&>/dev/null";
  if (spdlog::get_level() <= spdlog::level::debug) {
    redir = "";
  }

  auto unzip_command = fmt::format("cd {} && unzip submission.zip {}",
                                   record_location.native(), redir);

  spdlog::debug("  unzipping: system({})", unzip_command);

  auto errc = std::system(unzip_command.c_str());
  if (errc) {
    throw std::runtime_error(
        fmt::format("unzip command reported error: {}", errc));
  }
  std::filesystem::remove(download_location);

  if (std::filesystem::exists(expected_location)) {
    spdlog::debug("  resolved to: {}", expected_location.native());
    return expected_location;
  }

  // also check if the resource is the table name with a corresponding yaml file
  if (std::filesystem::exists(expected_location_yaml)) {
    spdlog::debug("  resolved to: {}", expected_location_yaml.native());
    return expected_location_yaml;
  }

  std::stringstream dir_contents;
  for (auto const &dir_entry :
       std::filesystem::directory_iterator{record_location}) {
    dir_contents << "  " << dir_entry.path().native() << '\n';
  }

  throw std::runtime_error(fmt::format(
      "After downloading and unzipping, failed to resolve "
      "reference: {} to a file in directory: {}, with contents:\n{}\n\nIs the "
      "resourcename, {}, correct?",
      ref.str(), record_location.native(), dir_contents.str(),
      ref.resourcename));
}

ResourceReference resolve_version(ResourceReference ref) {
  if (ref.reftype == "path") {
    return ref;
  }

  if (!ref.recordvers) { // unqualified version, check what the latest version
                         // is
    cpr::Url Endpoint = get_record_endpoint(ref);

    spdlog::debug("Checking latest version for unversioned ref={}", ref.str());
    spdlog::debug("  GET {}", Endpoint.str());

    cpr::Response r = cpr::Get(Endpoint, cpr::Parameters{{"format", "json"}});

    spdlog::debug("   http response --> {} ", r.status_code);

    if (r.status_code != 200) {
      throw std::runtime_error(
          fmt::format("GET response code: {}", r.status_code));
    }

    if (r.header["content-type"] != "application/json") {
      throw std::runtime_error(fmt::format(
          "GET response content-type: {}, expected \"application/json\"",
          r.header["content-type"]));
    }

    auto respdoc = YAML::Load(r.text);

    ref.recordvers = respdoc["version"].as<int>();
    spdlog::debug("  resolved reference with concrete version to: {}",
                  ref.str());
  }

  return ref;
}

std::filesystem::path
resolve_reference_HEPData(ResourceReference ref,
                          std::filesystem::path const &local_cache_root) {

  if (ref.reftype == "inspirehep") {
    return ensure_local_path(ref, local_cache_root);
  }

  ref = resolve_version(ref);

  return ensure_local_path(ref, local_cache_root);
}

std::filesystem::path
resolve_reference(ResourceReference const &ref,
                  std::filesystem::path const &local_cache_root) {

  spdlog::trace("resolve_reference(ref={},local_cache_root={})", ref.str(),
                local_cache_root.native());

  if (ref.reftype == "path") {
    auto resource_path = ref.path;

    if (ref.resourcename.size()) {
      resource_path /= ref.resourcename;
    } else if(std::filesystem::exists(resource_path / "submission.yaml")){
      return resource_path / "submission.yaml";
    }

    if (!std::filesystem::exists(resource_path)) {

      if(std::filesystem::exists(resource_path.native() + ".yaml")){
        return resource_path.native() + ".yaml";
      }

      throw std::runtime_error(
          fmt::format("Resolving a path-type reference: {} to path: {}, which "
                      "does not exist.",
                      ref.str(), resource_path.native()));
    }
    return resource_path;
  }

  return resolve_reference_HEPData(ref, local_cache_root);
}

} // namespace nuis::HEPData