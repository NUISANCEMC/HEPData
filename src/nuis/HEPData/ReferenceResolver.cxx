#include "nuis/HEPData/ReferenceResolver.h"

#include "cpr/cpr.h"

#include "yaml-cpp/yaml.h"

#include "fmt/core.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <fstream>
#include <iostream>

namespace nuis::HEPData {

static std::shared_ptr<spdlog::logger> refresolv_logger = nullptr;

spdlog::logger &refresolv_log() {
  if (!refresolv_logger) {
    refresolv_logger = spdlog::stdout_color_mt("NHPD-RefResolv");
    refresolv_logger->set_pattern("[NHPD RefResolv:%L]: %v");
  }
  return *refresolv_logger;
}

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

  refresolv_log().debug(
      R"(   * ensure_local_path for {} (local_cache_root={}))", ref.str(),
      local_cache_root.native());
  refresolv_log().debug(R"(   * expected resource location = {}{})",
                        expected_location.native(), yaml_opt);

  if (std::filesystem::exists(expected_location)) {
    refresolv_log().debug("   *-> expected resource location exists: {}",
                          expected_location.native());
    return expected_location;
  }

  auto expected_location_yaml = expected_location;
  expected_location_yaml += ".yaml";
  // also check if the resource is the table name with a corresponding yaml file
  if (std::filesystem::exists(expected_location_yaml)) {
    refresolv_log().debug(
        "   *-> expected resource location with .yaml extension exists: {}",
        expected_location.native());
    return expected_location_yaml;
  }

  auto record_location = get_expected_record_location(ref, local_cache_root);

  refresolv_log().debug(
      "   * Failed to directly resolve to a resource, checking "
      "expected record location: {}",
      record_location.native());

  // if the submission exists, then it is likely that this resource is mispelled
  if (std::filesystem::exists(record_location)) {
    std::stringstream dir_contents;
    for (auto const &dir_entry :
         std::filesystem::directory_iterator{record_location}) {
      dir_contents << "  " << dir_entry.path().native() << '\n';
    }

    throw std::runtime_error(fmt::format(
        "Failed to resolve reference: {} to a file in the record "
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

  refresolv_log().debug("   * No local copy of the record found");
  refresolv_log().debug("     * Try to fetch remote reference:");
  refresolv_log().debug("       * GET {} -> {} ", Endpoint.str(),
                        download_location.native());

  cpr::Response r =
      cpr::Download(of, Endpoint, cpr::Parameters{{"format", "original"}});

  refresolv_log().debug("       * http response code: {} ", r.status_code);

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
  if (refresolv_log().level() <= spdlog::level::debug) {
    redir = "";
  }

  auto unzip_command = fmt::format("cd {} && unzip submission.zip {}",
                                   record_location.native(), redir);

  refresolv_log().debug("     * unzipping: system({})", unzip_command);

  auto errc = std::system(unzip_command.c_str());
  if (errc) {
    throw std::runtime_error(
        fmt::format("unzip command reported error: {}", errc));
  }
  std::filesystem::remove(download_location);

  if (std::filesystem::exists(expected_location)) {
    refresolv_log().debug("   *-> resolved to newly downloaded file: {}",
                          expected_location.native());
    return expected_location;
  }

  // also check if the resource is the table name with a corresponding yaml file
  if (std::filesystem::exists(expected_location_yaml)) {
    refresolv_log().debug(
        "    *-> resolved to newly downloaded file with .yaml extension: {}",
        expected_location_yaml.native());
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

    refresolv_log().debug(
        "    * Checking latest version for unversioned ref={}", ref.str());
    refresolv_log().debug("      * GET {}", Endpoint.str());

    cpr::Response r = cpr::Get(Endpoint, cpr::Parameters{{"format", "json"}});

    refresolv_log().debug("      * http response --> {} ", r.status_code);

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
    refresolv_log().debug(
        "    *-> resolved reference with concrete version to: {}", ref.str());
  }

  return ref;
}

std::filesystem::path
resolve_reference_HEPData(ResourceReference ref,
                          std::filesystem::path const &local_cache_root) {

  refresolv_log().debug(R"(  * remote reference resolution)");

  if (ref.reftype == "inspirehep") {
    refresolv_log().debug(
        R"(  * inspirehep type reference must exist in local cache)");
    return ensure_local_path(ref, local_cache_root);
  }

  ref = resolve_version(ref);

  return ensure_local_path(ref, local_cache_root);
}

std::filesystem::path
resolve_reference(ResourceReference const &ref,
                  std::filesystem::path const &local_cache_root) {

  refresolv_log().debug(R"(* resolve_reference: {} (local_cache_root={}))",
                        ref.str(), local_cache_root.native());

  if (ref.reftype == "path") {
    auto resource_path = ref.path;

    refresolv_log().debug(R"(  * path type reference resolution)");

    if (ref.resourcename.size()) {
      resource_path /= ref.resourcename;
    } else if (std::filesystem::exists(resource_path / "submission.yaml")) {
      refresolv_log().debug(R"(  *-> resolved to existing path: {})",
                            (resource_path / "submission.yaml").native());
      return resource_path / "submission.yaml";
    }

    if (!std::filesystem::exists(resource_path)) {

      if (std::filesystem::exists(resource_path.native() + ".yaml")) {
        refresolv_log().debug(R"(  *-> resolved to existing path: {}.yaml)",
                              resource_path.native());
        return resource_path.native() + ".yaml";
      }

      refresolv_log().critical(R"(  * expected path does not exist: {})",
                               resource_path.native());

      throw std::runtime_error(
          fmt::format("Resolving a path-type reference: {} to path: {}, which "
                      "does not exist.",
                      ref.str(), resource_path.native()));
    }
    refresolv_log().debug(R"(  *-> resolved to existing path: {})",
                          resource_path.native());
    return resource_path;
  }

  return resolve_reference_HEPData(ref, local_cache_root);
}

} // namespace nuis::HEPData
