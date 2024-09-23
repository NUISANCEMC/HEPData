#include "nuis/ReferenceResolver.hxx"

#include "cpr/cpr.h"

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"

#include "spdlog/spdlog.h"

#include <fstream>
#include <iostream>

namespace nuis {

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

  spdlog::info("ensure_local_path({},{}): expected_location = {}", ref.str(),
               local_cache_root.native(), expected_location.native());

  if (std::filesystem::exists(expected_location)) {
    return expected_location;
  }

  auto expected_location_yaml = expected_location;
  expected_location_yaml += ".yaml";
  // also check if the resource is the table name with a corresponding yaml file
  if (std::filesystem::exists(expected_location_yaml)) {
    return expected_location_yaml;
  }

  if (ref.reftype == "inspirehep") {
    throw std::runtime_error(
        "Cannot yet fetch non-local inspirehep-type resources.");
  }

  std::filesystem::path download_location =
      fmt::format("{}/submission.zip",
                  get_expected_record_location(ref, local_cache_root).native());

  auto download_dir = download_location.parent_path();

  std::filesystem::create_directories(download_dir);

  std::ofstream of(download_location, std::ios::binary);

  cpr::Url Endpoint = get_record_endpoint(ref);

  spdlog::info("Doesn't exist, downloading...");
  spdlog::info("  GET {} -> {} ", Endpoint.str(), download_location.native());

  cpr::Response r =
      cpr::Download(of, Endpoint, cpr::Parameters{{"format", "original"}});

  spdlog::info("   --> {} ", r.status_code);

  if (r.status_code != 200) {
    throw std::runtime_error(
        fmt::format("GET response code: {}", r.status_code));
  }

  if (r.header["content-type"] != "application/zip") {
    throw std::runtime_error(fmt::format(
        "GET response content-type: {}, expected \"application/zip\"",
        r.header["content-type"]));
  }

  auto unzip_command =
      fmt::format("cd {} && unzip submission.zip", download_dir.native());

  spdlog::info("  unzipping: system({})", unzip_command);

  auto errc = std::system(unzip_command.c_str());
  if (errc) {
    throw std::runtime_error(
        fmt::format("unzip command reported error: {}", errc));
  }
  std::filesystem::remove(download_location);

  // if (std::filesystem::exists(expected_location)) {
  //   throw std::runtime_error(
  //       fmt::format("After fetching and unpacking the response from {}, could "
  //                   "not find expected file {}.",
  //                   Endpoint.str(), expected_location.native()));
  // }

  spdlog::info("  resolved to: {}", expected_location.native());

  return expected_location;
}

std::filesystem::path
resolve_reference_HEPData(ResourceReference ref,
                          std::filesystem::path const &local_cache_root) {

  if (ref.reftype == "inspirehep") {
    return ensure_local_path(ref, local_cache_root);
  }

  if (!ref.recordvers) { // unqualified version, check what the latest version
                         // is
    cpr::Url Endpoint = get_record_endpoint(ref);

    spdlog::info("Checking latest version for {}", ref.str());
    spdlog::info("  GET {}", Endpoint.str());

    cpr::Response r = cpr::Get(Endpoint, cpr::Parameters{{"format", "json"}});

    spdlog::info("   --> {} ", r.status_code);

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
    spdlog::info("  resolved reference with concrete version to: {}",
                 ref.str());
  }

  return ensure_local_path(ref, local_cache_root);
}

std::filesystem::path
resolve_reference(ResourceReference const &ref,
                  std::filesystem::path const &local_cache_root) {

  spdlog::info("resolve_reference({},{})", ref.str(),
               local_cache_root.native());

  return resolve_reference_HEPData(ref, local_cache_root);
}

} // namespace nuis