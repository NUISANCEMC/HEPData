#pragma once

#include "nuis/HEPData/HEPDataRecord.hxx"

#include "fmt/core.h"

#include <iostream>

namespace nuis {

std::ostream &operator<<(std::ostream &os, HEPDataExtent const &ext) {
  return os << fmt::format("{{ low: {}, high: {} }}", ext.low, ext.high);
}

std::ostream &operator<<(std::ostream &os, HEPDataValue const &val) {
  if (val.value.index() == 0) {
    return os << "{ v: " << std::get<0>(val.value) << " }";
  } else {
    return os << "{ v: " << std::get<1>(val.value) << " }";
  }
}

std::ostream &operator<<(std::ostream &os, HEPDataVariable const &var) {
  os << fmt::format("name: \"{}\", units: \"{}\"\n  values:\n", var.name,
                    var.units);
  for (auto const &v : var.values) {
    os << "    " << v << "\n";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         HEPDataDependentVariable const &var) {
  os << static_cast<HEPDataVariable const &>(var);
  os << "  qualifiers:\n";
  for (auto const &q : var.qualifiers) {
    os << "    \"" << q.first << "\": \"" << q.second << "\"\n";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, HEPDataTable const &tbl) {
  os << "---\nindependent variables:\n";
  for (auto const &v : tbl.independent_vars) {
    os << v << "\n---\n";
  }

  os << "---\ndependent variables:\n";
  for (auto const &v : tbl.dependent_vars) {
    os << v << "\n---\n";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, HEPDataProbeFlux const &pf) {

  os << "probe_particle: " << pf.probe_particle << "\n";
  os << "bin_content_type: " << pf.bin_content_type << "\n";

  os << static_cast<HEPDataTable const &>(pf);

  return os;
}

std::ostream &operator<<(std::ostream &os,
                         HEPDataCrossSectionMeasurement::funcref const &fref) {
  return os << "{" << fref.fname << " from " << fref.source << "}";
}

std::ostream &operator<<(std::ostream &os,
                         HEPDataCrossSectionMeasurement const &measurement) {

  os << "---\nprobe fluxes:\n";
  for (auto const &pfs : measurement.probe_fluxes) {
    for (auto const &pf : pfs) {
      os << "\n+++\n";
      os << "  weight: " << pf.weight << "\n  " << (*pf) << "\n+++\n";
      os << "\n+++\n";
    }
    os << "\n---\n";
  }

  os << "---\ntargets:\n";
  for (auto const &tgts : measurement.targets) {
    for (auto const &tgt : tgts) {
      os << "  weight: " << tgt.weight << ", target: " << (*tgt) << "\n+++\n";
    }
    os << "\n---\n";
  }

  os << "selection functions: \n";
  for (auto const &sf : measurement.selectfuncs) {
    os << "  " << sf << "\n";
  }

  os << "projectfuncs:" << "\n";
  for (size_t i = 0; i < measurement.projectfuncs.size(); ++i) {
    os << "  ---[" << i << "]:\n";
    for (size_t j = 0; j < measurement.independent_vars.size(); ++j) {
      os << "    " << measurement.independent_vars[j].name << ": "
         << measurement.projectfuncs[i][j] << "\n";
    }
  }

  os << static_cast<HEPDataTable const &>(measurement);

  return os;
}

std::ostream &operator<<(std::ostream &os, HEPDataRecord const &rec) {
  os << "+++\ncross section measurements:\n";
  for (auto const &m : rec.measurements) {
    os << m << "\n+++\n";
  }
  return os;
}

} // namespace nuis