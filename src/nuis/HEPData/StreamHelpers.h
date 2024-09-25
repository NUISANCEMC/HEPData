#pragma once

#include "nuis/HEPData/Record.h"

#include <iostream>

namespace nuis::HEPData {

std::ostream &operator<<(std::ostream &os, Extent const &ext);

std::ostream &operator<<(std::ostream &os, Value const &val);

std::ostream &operator<<(std::ostream &os, Variable const &var);

std::ostream &operator<<(std::ostream &os, DependentVariable const &var);

std::ostream &operator<<(std::ostream &os, Table const &tbl);

std::ostream &operator<<(std::ostream &os, ProbeFlux const &pf);

std::ostream &operator<<(std::ostream &os,
                         CrossSectionMeasurement::funcref const &fref);

std::ostream &operator<<(std::ostream &os,
                         CrossSectionMeasurement::Target const &tgt);

std::ostream &operator<<(std::ostream &os,
                         CrossSectionMeasurement const &measurement);

std::ostream &operator<<(std::ostream &os, Record const &rec);

} // namespace nuis::HEPData