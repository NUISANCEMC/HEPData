#include "nuis/HEPData/ResourceReference.h"

#include "fmt/core.h"

#include "spdlog/spdlog.h"

#include <filesystem>

namespace nuis::HEPData {

ResourceReference const HEPDataRef{"hepdata:/"};

// ref format: [<type=hepdata>:][<id>][[/]<resource[:<qualifier>]>]
ResourceReference::ResourceReference(std::string const &ref,
                                     ResourceReference const &context) {
  // dont blindly take context of path-type references
  if (context.reftype != "path") {
    *this = context;
    valid = true;

    refstr = ref;
    context_refstr = context.refstr;
  }

  if (!ref.size()) {
    return;
  }

  // '/' is the unique delimiter, search for that first
  auto fslash_pos = ref.find_first_of('/');

  // have to be careful about things if we don't have a 'full' reference
  if (fslash_pos == std::string::npos) {
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
      // if there is a colon, then the idv should be after the colon
      if (parse_idv(ref.substr(colon_pos + 1))) {
        parse_typeidv(ref);
        return;
      }
    }

    // try parsing the whole thing as an idv
    if (parse_idv(ref)) {
      parse_typeidv(ref);
      return;
    } else { // assume this is a resourcequal
      parse_resourcequal(ref);

      // if we only specify a resourcequal for this reference, then we should
      // take the path context
      if (context.reftype == "path") {
        reftype = "path";
        context_refstr = context.refstr;

        std::filesystem::path refpath = context.refstr;

        // if the refpath has submission.yaml on the end, remove it
        if (refpath.filename() == "submission.yaml") {
          refpath = refpath.parent_path();
          if (!resourcename.size()) {
            resourcename = "submission.yaml";
          }
        }

        // set the path and test
        refstr = refpath.native();

        if (resourcename.size()) {
          refpath /= resourcename;
        }

        if (!std::filesystem::exists(refpath) &&
            (refpath.extension() != ".yaml") &&
            std::filesystem::exists(refpath.native() + ".yaml")) {
          resourcename += ".yaml";
          refpath += ".yaml";
        }

        if (!std::filesystem::exists(refpath)) {
          throw std::runtime_error(fmt::format(
              "When building ResourceReference from path-type "
              "reference with path:{}, resourcename: {}, file: {}[.yaml]"
              "does not exist.",
              refpath.native(), resourcename,
              (refpath / resourcename).native()));
        }

        spdlog::debug(
            "building pathtype ref: {} from refstr: {}, and context: {}",
            this->str(), ref, context.str());

      } // end if (context.reftype == "path")

      return;
    }
  }

  parse_typeidv(ref.substr(0, fslash_pos));
  parse_resourcequal(ref.substr(fslash_pos + 1));
}

std::optional<std::tuple<size_t, int>>
ResourceReference::parse_idv(std::string idv) {
  if (!idv.size()) {
    return std::nullopt;
  }

  size_t _recordid = 0;
  int _recordvers = 0;

  // 'v' is the delimiter, search for that
  auto vpos = idv.find_first_of('v');
  if (vpos != std::string::npos) {
    try {
      _recordvers = std::stoi(idv.substr(vpos + 1));
    } catch (...) {
      return std::nullopt;
    }
    idv = idv.substr(0, vpos);
  }

  try {
    _recordid = std::stoll(idv);
  } catch (...) {
    return std::nullopt;
  }

  return std::make_tuple(_recordid, _recordvers);
}

void ResourceReference::parse_typeidv(std::string typeidv) {
  if (!typeidv.size()) {
    return;
  }

  // if we are overwriting the record, then flatten any context more specific
  resourcename = "";
  qualifier = "";

  // ':' is the delimiter, search for that
  auto colon_pos = typeidv.find_first_of(':');
  if (colon_pos != std::string::npos) {
    reftype = typeidv.substr(0, colon_pos);
    typeidv = typeidv.substr(colon_pos + 1);
  }

  auto idv = parse_idv(typeidv);
  if (idv) {
    std::tie(recordid, recordvers) = idv.value();
  } else {
    valid = false;
  }
}

void ResourceReference::parse_resourcequal(std::string resourceref) {
  if (!resourceref.size()) {
    return;
  }

  // if we are overwriting the resource, then flatten any context more specific
  qualifier = "";

  // ':' is the delimiter, search for that
  auto colon_pos = resourceref.find_first_of(':');
  if (colon_pos != std::string::npos) {
    qualifier = resourceref.substr(colon_pos + 1);
    resourceref = resourceref.substr(0, colon_pos);
  }
  resourcename = resourceref;
}

ResourceReference ResourceReference::record_ref() const {

  if (reftype == "path") {
    ResourceReference ref;
    ref.reftype = reftype;
    if (std::filesystem::path(refstr).filename().native() !=
        "submission.yaml") {
      ref.resourcename = "submission.yaml";
    }
    ref.refstr = refstr;
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
    ss << "path:[" << refstr << "]";
  } else {

    ss << reftype << ":" << recordid;

    if (recordvers) {
      ss << "v" << recordvers;
    }
  }

  if (resourcename.size()) {
    ss << "/" << resourcename;
  }

  if (qualifier.size()) {
    ss << ":" << qualifier;
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

} // namespace nuis::HEPData