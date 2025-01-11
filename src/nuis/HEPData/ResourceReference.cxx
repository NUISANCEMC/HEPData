#include "nuis/HEPData/ResourceReference.h"

#include "fmt/core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <filesystem>

namespace nuis::HEPData {

static std::shared_ptr<spdlog::logger> ref_logger = nullptr;

spdlog::logger &ref_log() {
  if (!ref_logger) {
    ref_logger = spdlog::stdout_color_mt("NHPD-Ref");
    ref_logger->set_pattern("[NHPD       Ref:%L]: %v");
  }
  return *ref_logger;
}

// ref format: [<type=hepdata>:][<id>][[/]<resource[:<qualifier>]>]
ResourceReference::ResourceReference(std::string const &ref,
                                     ResourceReference const &context) {

  ref_log().debug("| Building ResourceReference from ref={}, with context={}",
                  ref, context.str());

  valid = true;

  // dont blindly take context of path-type references
  if (context.reftype != "path") {
    ref_log().debug(
        "  | context is not a path-type reference, so taking context");
    *this = context;

    refstr = ref;
    context_refstr = context.refstr;
  }

  if (!ref.size()) {
    ref_log().debug("  |-> ref string is empty. Returning: {}", str());
    return;
  }

  // '/' is the unique delimiter, search for that first
  auto fslash_pos = ref.find_first_of('/');

  // have to be careful about things if we don't have a 'full' reference
  if (fslash_pos == std::string::npos) {
    ref_log().debug("  | no forward slash in reference");

    // if there is no '/' then its either a typeidv or a resourcequal
    auto colon_pos = ref.find_first_of(':');
    if (colon_pos != std::string::npos) {
      if (ref.find_first_of(':', colon_pos + 1) != std::string::npos) {
        throw std::runtime_error(
            fmt::format("reference: {} contained two colons but no forward "
                        "slash, this is not a valid reference of the format: "
                        "[<type=hepdata>:][<id>][[/]<resource[:<qualifier>]>].",
                        ref));
      }
      ref_log().debug(
          "  | checking if string after colon is parseable as an id[v]: {}",
          ref.substr(colon_pos + 1));
      // if there is a colon, then the idv should be after the colon
      if (parse_idv(ref.substr(colon_pos + 1))) {
        parse_typeidv(ref);
        ref_log().debug("  |-> ref: {}", str());
        return;
      }
    }

    ref_log().debug("  | checking if whole ref is parseable as an id[v]: {}",
                    ref);
    // try parsing the whole thing as an idv
    if (parse_idv(ref)) {
      parse_typeidv(ref);
      ref_log().debug("  |-> ref: {}", str());
      return;
    } else { // assume this is a resourcequal
      ref_log().debug(
          "  | Cannot find reference id. Assuming reference is to a "
          "resource on the context record");
      parse_resourcequal(ref);

      // if we only specify a resourcequal for this reference, then we should
      // take the path context
      if (context.reftype == "path") {
        ref_log().debug("  | context is path-type, so copy path to record");
        reftype = "path";
        path = context.path;
      }

      ref_log().debug("  |-> ref: {}", str());
      return;
    }
  }
  ref_log().debug("  | have forward slash");

  parse_typeidv(ref.substr(0, fslash_pos));
  parse_resourcequal(ref.substr(fslash_pos + 1));

  if (reftype == "path") {
    ref_log().debug("  | for path-type reference, checking validity");
    if (!std::filesystem::exists(path / resourcename) &&
        !std::filesystem::exists(path / (resourcename + ".yaml"))) {
      ref_log().debug("  | path: {}[.yaml] does not exist.",
                      (path / resourcename).native());
      valid = false;
    }
  }

  ref_log().debug("  |-> ref: {}", str());
}

std::optional<std::tuple<size_t, int>>
ResourceReference::parse_idv(std::string idv) {
  ref_log().debug("  | parsing idv string: {}", idv);
  if (!idv.size()) {
    ref_log().debug("    | empty string, invalid idv");
    return std::nullopt;
  }

  size_t _recordid = 0;
  int _recordvers = 0;

  // 'v' is the delimiter, search for that
  auto vpos = idv.find_first_of('v');
  if (vpos != std::string::npos) {
    ref_log().debug(
        "    | found 'v', may be id, try parse version component {} as integer",
        idv.substr(vpos + 1));
    try {
      _recordvers = std::stoi(idv.substr(vpos + 1));
    } catch (...) {
      ref_log().debug("    | invalid idv");
      return std::nullopt;
    }
    idv = idv.substr(0, vpos);
  }

  ref_log().debug("    | try parse id component {} as integer", idv);
  try {
    _recordid = std::stoll(idv);
  } catch (...) {
    ref_log().debug("    | invalid idv");
    return std::nullopt;
  }

  ref_log().debug("    | parsed valid idv: id: {}, version: {}", _recordid,
                  _recordvers);
  return std::make_tuple(_recordid, _recordvers);
}

void ResourceReference::parse_typeidv(std::string typeidv) {
  ref_log().debug("  | parsing typeidv string: {}", typeidv);
  if (!typeidv.size()) {
    ref_log().debug("    | empty string");
    return;
  }

  ref_log().debug("    | parsing new typeidv, removing any existing context: "
                  "resource={}, qualifier={}",
                  resourcename, qualifier);

  // if we are overwriting the record, then flatten any context more specific
  resourcename = "";
  qualifier = "";
  path = "";

  // ':' is the delimiter, search for that
  auto colon_pos = typeidv.find_first_of(':');
  if (colon_pos != std::string::npos) {
    ref_log().debug("  | found colon, type: {}", typeidv.substr(0, colon_pos));
    reftype = typeidv.substr(0, colon_pos);
    typeidv = typeidv.substr(colon_pos + 1);
  }

  auto idv = parse_idv(typeidv);
  if (idv) {
    std::tie(recordid, recordvers) = idv.value();
  } else {
    ref_log().debug("    | invalid typeidv");
    valid = false;
  }
}

void ResourceReference::parse_resourcequal(std::string resourceref) {
  ref_log().debug("  | parsing resourcequal string: {}", resourceref);
  if (!resourceref.size()) {
    ref_log().debug("    | empty string");
    return;
  }

  ref_log().debug(
      "  | parsing resource, removing existing context, qualifier={}",
      qualifier);

  // if we are overwriting the resource, then flatten any context more
  // specific
  qualifier = "";

  // ':' is the delimiter, search for that
  auto colon_pos = resourceref.find_first_of(':');
  if (colon_pos != std::string::npos) {
    ref_log().debug("    | found colon, extracting qualifier: {}",
                    resourceref.substr(colon_pos + 1));
    qualifier = resourceref.substr(colon_pos + 1);
    resourceref = resourceref.substr(0, colon_pos);
  }
  resourcename = resourceref;
  ref_log().debug("    | extracting resource: {}", resourcename);
}

ResourceReference ResourceReference::record_ref() const {

  if (reftype == "path") {
    ResourceReference ref;
    ref.reftype = reftype;
    ref.path = path;
    return ref;
  }

  std::stringstream ss;

  if (!recordid) {
    throw std::runtime_error(
        fmt::format("Request for record_ref from ResourceReference({}) which "
                    "had no recordid.",
                    str()));
  }

  ss << reftype << ":" << recordid;

  if (recordvers) {
    ss << "v" << recordvers;
  }

  return ResourceReference(ss.str());
}

std::string ResourceReference::str() const {

  if (!valid) {
    return fmt::format("INVALID-REFERENCE:[{}, context:{}]", refstr,
                       context_refstr);
  }

  std::stringstream ss;

  if (reftype == "path") {

    ss << "path:" << path.native() << "";
    if (resourcename.size()) {
      ss << ":" << resourcename;
    }

    if (qualifier.size()) {
      ss << ":" << qualifier;
    }

  } else {

    ss << reftype << ":" << recordid;

    if (recordvers) {
      ss << "v" << recordvers;
    }
    if (resourcename.size()) {
      ss << "/" << resourcename;
    }

    if (qualifier.size()) {
      ss << ":" << qualifier;
    }
  }

  return ss.str();
}

std::string ResourceReference::component(std::string const &comp) const {
  if ((comp == "reftype") || (comp == "type")) {
    return reftype;
  } else if ((comp == "recordid") || (comp == "id")) {
    return std::to_string(recordid);
  } else if ((comp == "recordvers") || (comp == "versions")) {
    return std::to_string(recordvers);
  } else if ((comp == "resourcename") || (comp == "resource")) {
    return resourcename;
  } else if (comp == "qualifier") {
    return qualifier;
  }
  throw std::runtime_error(
      fmt::format("Invalid reference component requested: {}", comp));
}

// ref format: /path/to/file.yaml[:<resource[:qualifier]>]
ResourceReference PathResourceReference(std::string const &refstr) {
  ResourceReference out_ref;
  out_ref.valid = true;

  ref_log().debug("| Building path-type reference: {}", refstr);

  out_ref.reftype = "path";

  auto first_colon = refstr.find_first_of(':');
  if (first_colon != std::string::npos) {

    out_ref.path = refstr.substr(0, first_colon);

    ref_log().debug("  | have colon, path = {}", out_ref.path.native());

    auto second_colon = refstr.find_first_of(':', first_colon + 1);
    if (second_colon != std::string::npos) {

      out_ref.resourcename =
          refstr.substr(first_colon + 1, second_colon - (first_colon + 1));
      out_ref.qualifier = refstr.substr(second_colon + 1);
      ref_log().debug("  | have second colon, resource = {}, qualifier = {}",
                      out_ref.resourcename, out_ref.qualifier);
    } else {
      out_ref.resourcename = refstr.substr(first_colon + 1);
      ref_log().debug("  | resource = {}", out_ref.resourcename);
    }

  } else {
    ref_log().debug("  | reference is pure path");
    out_ref.path = refstr;
  }

  ref_log().debug("  | check reference is valid");

  auto fstatus = std::filesystem::status(out_ref.path);
  if (!std::filesystem::exists(fstatus)) {
    throw std::runtime_error(fmt::format(
        "PathResourceReference({}) resolves to a non-existant file.", refstr));
  }

  if (!std::filesystem::is_directory(
          fstatus)) { // if its not a directory and points at anything except
                      // a submission.yaml and there is also a resourcename,
                      // then we're pointing at two files at once.
    ref_log().debug("  | reference does not point to a directory");

    if (out_ref.path.filename() == "submission.yaml") {
      ref_log().debug(
          "      | it points directly to a submission.yaml, set the parent "
          "path as the path and the submission.yaml as the resource.");
      out_ref.path = out_ref.path.parent_path();
    } else if (out_ref.resourcename.size()) {
      throw std::runtime_error(
          fmt::format("PathResourceReference({}) resolves to a file={}, but "
                      "resourcename={}, is also set.",
                      refstr, out_ref.path.native(), out_ref.resourcename));
    }
  } else { // is directory
    ref_log().debug("  | reference does point to a directory");
    if (!out_ref.resourcename
             .size()) { // if there is no file, check if submission.yaml
                        // exists, in which case this is a valid reference to
                        // a record.
      if (!std::filesystem::exists(out_ref.path / "submission.yaml")) {
        throw std::runtime_error(fmt::format(
            "PathResourceReference({}) resolves to a directory={}, but "
            "resourcename is not set and {} does not exist.",
            refstr, out_ref.path.native(),
            (out_ref.path / "submission.yaml").native()));
      }
      ref_log().debug("  | directory contains submission.yaml");

    } else { // check if that file exists
      if (!std::filesystem::exists(out_ref.path / out_ref.resourcename)) {
        if (std::filesystem::exists(out_ref.path /
                                    (out_ref.resourcename + ".yaml"))) {
          ref_log().debug("  |-> {}", out_ref.str());
          return out_ref;
        }
        throw std::runtime_error(fmt::format(
            "PathResourceReference({}) resolves to a directory={}, with "
            "resourcename={} set, but file={} does not exist.",
            refstr, out_ref.path.native(), out_ref.resourcename,
            (out_ref.path / out_ref.resourcename).native()));
      }
    }
  }
  ref_log().debug("  |-> {}", out_ref.str());

  return out_ref;
}

} // namespace nuis::HEPData
