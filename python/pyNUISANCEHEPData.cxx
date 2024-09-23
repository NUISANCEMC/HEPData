#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "nuis/HEPData/HEPDataTableFactory.hxx"
#include "nuis/HEPData/ReferenceResolver.hxx"
#include "nuis/HEPData/ResourceReference.hxx"
#include "nuis/HEPData/StreamHelpers.hxx"

namespace py = pybind11;
using namespace nuis;

PYBIND11_MODULE(pyNUISANCEHEPData, m) {
  m.doc() = "pyNUISANCEHEPData implementation in python";

  py::class_<std::filesystem::path>(m, "fsPath")
      .def(py::init<std::string>())
      .def("__str__",
           [](std::filesystem::path const &p) { return p.native(); });
  py::implicitly_convertible<std::string, std::filesystem::path>();

  py::class_<HEPDataExtent>(m, "HEPDataExtent")
      .def_readonly("low", &HEPDataExtent::low)
      .def_readonly("high", &HEPDataExtent::high);

  py::class_<HEPDataValue>(m, "HEPDataValue")
      .def_readonly("value", &HEPDataValue::value)
      .def_readonly("errors", &HEPDataValue::errors);

  py::class_<HEPDataVariable>(m, "HEPDataVariable")
      .def_readonly("values", &HEPDataVariable::values)
      .def_readonly("name", &HEPDataVariable::name)
      .def_readonly("units", &HEPDataVariable::units);

  py::class_<HEPDataDependentVariable, HEPDataVariable>(
      m, "HEPDataDependentVariable")
      .def_readonly("qualifiers", &HEPDataDependentVariable::qualifiers);

  py::class_<HEPDataTable>(m, "HEPDataTable")
      .def_readonly("source", &HEPDataTable::source)
      .def_readonly("independent_vars", &HEPDataTable::independent_vars)
      .def_readonly("dependent_vars", &HEPDataTable::dependent_vars)
      .def("__str__", [](HEPDataTable const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPDataProbeFlux, HEPDataTable>(m, "HEPDataProbeFlux")
      .def_readonly("probe_particle", &HEPDataProbeFlux::probe_particle)
      .def_readonly("bin_content_type", &HEPDataProbeFlux::bin_content_type)
      .def("__str__", [](HEPDataProbeFlux const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPDataErrorTable, HEPDataTable>(m, "HEPDataErrorTable")
      .def_readonly("error_type", &HEPDataErrorTable::error_type)
      .def("__str__", [](HEPDataErrorTable const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPDataSmearingTable, HEPDataTable>(m, "HEPDataSmearingTable")
      .def("__str__", [](HEPDataSmearingTable const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPDataCrossSectionMeasurement::funcref>(
      m, "HEPDataCrossSectionMeasurement::funcref")
      .def_readonly("source", &HEPDataCrossSectionMeasurement::funcref::source)
      .def_readonly("fname", &HEPDataCrossSectionMeasurement::funcref::fname);

  py::class_<HEPDataCrossSectionMeasurement::Weighted<HEPDataProbeFlux>>(
      m, "HEPDataCrossSectionMeasurement::Weighted<HEPDataProbeFlux>")
      .def_readonly(
          "obj",
          &HEPDataCrossSectionMeasurement::Weighted<HEPDataProbeFlux>::obj)
      .def_readonly(
          "weight",
          &HEPDataCrossSectionMeasurement::Weighted<HEPDataProbeFlux>::weight);

  py::class_<HEPDataCrossSectionMeasurement::Weighted<std::string>>(
      m, "HEPDataCrossSectionMeasurement::Weighted<std::string>")
      .def_readonly("obj",
                    &HEPDataCrossSectionMeasurement::Weighted<std::string>::obj)
      .def_readonly(
          "weight",
          &HEPDataCrossSectionMeasurement::Weighted<std::string>::weight);

  py::class_<HEPDataCrossSectionMeasurement, HEPDataTable>(
      m, "HEPDataCrossSectionMeasurement")
      .def_readonly("probe_fluxes",
                    &HEPDataCrossSectionMeasurement::probe_fluxes)
      .def_readonly("targets", &HEPDataCrossSectionMeasurement::targets)
      .def_readonly("errors", &HEPDataCrossSectionMeasurement::errors)
      .def_readonly("smearings", &HEPDataCrossSectionMeasurement::smearings)
      .def_readonly("selectfuncs", &HEPDataCrossSectionMeasurement::selectfuncs)
      .def_readonly("projectfuncs",
                    &HEPDataCrossSectionMeasurement::projectfuncs)
      .def("__str__", [](HEPDataCrossSectionMeasurement const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPDataRecord>(m, "HEPDataRecord")
      .def_readonly("record_root", &HEPDataRecord::record_root)
      .def_readonly("record_ref", &HEPDataRecord::record_ref)
      .def_readonly("measurements", &HEPDataRecord::measurements)
      .def_readonly("additional_resources",
                    &HEPDataRecord::additional_resources)
      .def("__str__", [](HEPDataRecord const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<ResourceReference>(m, "ResourceReference")
      .def(py::init<std::string const &, ResourceReference const &>(),
           py::arg("ref"), py::arg("context") = ResourceReference())
      .def_readonly("reftype", &ResourceReference::reftype)
      .def_readonly("recordid", &ResourceReference::recordid)
      .def_readonly("recordvers", &ResourceReference::recordvers)
      .def_readonly("resourcename", &ResourceReference::resourcename)
      .def_readonly("qualifier", &ResourceReference::qualifier);

  m.def("make_HEPDataCrossSectionMeasurement",
        &make_HEPDataCrossSectionMeasurement, py::arg("ref"),
        py::arg("local_cache_root") = ".")
      .def(
          "make_HEPDataCrossSectionMeasurement",
          [](std::string const &ref,
             std::filesystem::path const &local_cache_root) {
            return make_HEPDataCrossSectionMeasurement(ResourceReference(ref),
                                                       local_cache_root);
          },
          py::arg("ref"), py::arg("local_cache_root") = ".")
      .def("make_HEPDataRecord", &make_HEPDataRecord, py::arg("ref"),
           py::arg("local_cache_root") = ".")
      .def(
          "make_HEPDataRecord",
          [](std::string const &ref,
             std::filesystem::path const &local_cache_root) {
            return make_HEPDataRecord(ResourceReference(ref), local_cache_root);
          },
          py::arg("ref"), py::arg("local_cache_root") = ".")
      .def("resolve_reference", &resolve_reference, py::arg("ref"),
           py::arg("local_cache_root") = ".")
      .def(
          "resolve_reference",
          [](std::string const &ref,
             std::filesystem::path const &local_cache_root) {
            return resolve_reference(ResourceReference(ref), local_cache_root);
          },
          py::arg("ref"), py::arg("local_cache_root") = ".");
}