#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "nuis/HEPData/ReferenceResolver.h"
#include "nuis/HEPData/ResourceReference.h"
#include "nuis/HEPData/StreamHelpers.h"
#include "nuis/HEPData/TableFactory.h"

namespace py = pybind11;
using namespace nuis;

PYBIND11_MODULE(pyNUISANCEHEPData, m) {
  m.doc() = "pyNUISANCEHEPData implementation in python";

  py::class_<std::filesystem::path>(m, "fsPath")
      .def(py::init<std::string>())
      .def("__str__",
           [](std::filesystem::path const &p) { return p.native(); });
  py::implicitly_convertible<std::string, std::filesystem::path>();

  py::class_<HEPData::Extent>(m, "HEPData::Extent")
      .def_readonly("low", &HEPData::Extent::low)
      .def_readonly("high", &HEPData::Extent::high);

  py::class_<HEPData::Value>(m, "HEPData::Value")
      .def_readonly("value", &HEPData::Value::value)
      .def_readonly("errors", &HEPData::Value::errors);

  py::class_<HEPData::Variable>(m, "HEPData::Variable")
      .def_readonly("values", &HEPData::Variable::values)
      .def_readonly("name", &HEPData::Variable::name)
      .def_readonly("units", &HEPData::Variable::units);

  py::class_<HEPData::DependentVariable, HEPData::Variable>(
      m, "HEPData::DependentVariable")
      .def_readonly("qualifiers", &HEPData::DependentVariable::qualifiers);

  py::class_<HEPData::Table>(m, "HEPData::Table")
      .def_readonly("source", &HEPData::Table::source)
      .def_readonly("independent_vars", &HEPData::Table::independent_vars)
      .def_readonly("dependent_vars", &HEPData::Table::dependent_vars)
      .def("__str__", [](HEPData::Table const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPData::ProbeFlux, HEPData::Table>(m, "HEPData::ProbeFlux")
      .def_readonly("probe_particle", &HEPData::ProbeFlux::probe_particle)
      .def_readonly("bin_content_type", &HEPData::ProbeFlux::bin_content_type)
      .def("__str__", [](HEPData::ProbeFlux const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPData::ErrorTable, HEPData::Table>(m, "HEPData::ErrorTable")
      .def_readonly("error_type", &HEPData::ErrorTable::error_type)
      .def("__str__", [](HEPData::ErrorTable const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPData::SmearingTable, HEPData::Table>(m,
                                                     "HEPData::SmearingTable")
      .def("__str__", [](HEPData::SmearingTable const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPData::CrossSectionMeasurement::funcref>(
      m, "HEPData::CrossSectionMeasurement::funcref")
      .def_readonly("source",
                    &HEPData::CrossSectionMeasurement::funcref::source)
      .def_readonly("fname", &HEPData::CrossSectionMeasurement::funcref::fname);

  py::class_<HEPData::CrossSectionMeasurement::Weighted<HEPData::ProbeFlux>>(
      m, "HEPData::CrossSectionMeasurement::Weighted<HEPData::ProbeFlux>")
      .def_readonly(
          "obj",
          &HEPData::CrossSectionMeasurement::Weighted<HEPData::ProbeFlux>::obj)
      .def_readonly("weight", &HEPData::CrossSectionMeasurement::Weighted<
                                  HEPData::ProbeFlux>::weight);

  py::class_<HEPData::CrossSectionMeasurement::Target>(
      m, "HEPData::CrossSectionMeasurement::Target")
      .def_readonly("A", &HEPData::CrossSectionMeasurement::Target::A)
      .def_readonly("Z", &HEPData::CrossSectionMeasurement::Target::Z);

  py::class_<HEPData::CrossSectionMeasurement::Weighted<
      HEPData::CrossSectionMeasurement::Target>>(
      m, "HEPData::CrossSectionMeasurement::Weighted<HEPData::"
         "CrossSectionMeasurement::Target>")
      .def_readonly("obj", &HEPData::CrossSectionMeasurement::Weighted<
                               HEPData::CrossSectionMeasurement::Target>::obj)
      .def_readonly("weight",
                    &HEPData::CrossSectionMeasurement::Weighted<
                        HEPData::CrossSectionMeasurement::Target>::weight);

  py::class_<HEPData::CrossSectionMeasurement, HEPData::Table>(
      m, "HEPData::CrossSectionMeasurement")
      .def_readonly("name", &HEPData::CrossSectionMeasurement::name)
      .def_readonly("probe_fluxes",
                    &HEPData::CrossSectionMeasurement::probe_fluxes)
      .def_readonly("targets", &HEPData::CrossSectionMeasurement::targets)
      .def_readonly("errors", &HEPData::CrossSectionMeasurement::errors)
      .def_readonly("smearings", &HEPData::CrossSectionMeasurement::smearings)
      .def_readonly("selectfuncs",
                    &HEPData::CrossSectionMeasurement::selectfuncs)
      .def_readonly("projectfuncs",
                    &HEPData::CrossSectionMeasurement::projectfuncs)
      .def_readonly("variable_type",
                    &HEPData::CrossSectionMeasurement::variable_type)
      .def_readonly("measurement_type",
                    &HEPData::CrossSectionMeasurement::measurement_type)
      .def_readonly("sub_measurements",
                    &HEPData::CrossSectionMeasurement::sub_measurements)
      .def_readonly("cross_section_units",
                    &HEPData::CrossSectionMeasurement::cross_section_units)
      .def_readonly("test_statistic",
                    &HEPData::CrossSectionMeasurement::test_statistic)
      .def("get_single_probe_flux",
           &HEPData::CrossSectionMeasurement::get_single_probe_flux)
      .def("get_simple_target",
           &HEPData::CrossSectionMeasurement::get_simple_target)
      .def("get_single_selectfunc",
           &HEPData::CrossSectionMeasurement::get_single_selectfunc)
      .def("get_single_projectfuncs",
           &HEPData::CrossSectionMeasurement::get_single_projectfuncs)
      .def("__str__", [](HEPData::CrossSectionMeasurement const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPData::Record>(m, "HEPData::Record")
      .def_readonly("record_root", &HEPData::Record::record_root)
      .def_readonly("record_ref", &HEPData::Record::record_ref)
      .def_readonly("measurements", &HEPData::Record::measurements)
      .def_readonly("additional_resources",
                    &HEPData::Record::additional_resources)
      .def("__str__", [](HEPData::Record const &hpd) {
        std::stringstream ss;
        ss << hpd;
        return ss.str();
      });

  py::class_<HEPData::ResourceReference>(m, "HEPData::ResourceReference")
      .def(py::init<std::string const &, HEPData::ResourceReference const &>(),
           py::arg("ref"), py::arg("context") = HEPData::ResourceReference())
      .def_readonly("reftype", &HEPData::ResourceReference::reftype)
      .def_readonly("recordid", &HEPData::ResourceReference::recordid)
      .def_readonly("recordvers", &HEPData::ResourceReference::recordvers)
      .def_readonly("resourcename", &HEPData::ResourceReference::resourcename)
      .def_readonly("qualifier", &HEPData::ResourceReference::qualifier);

  m.def("PathResourceReference", &HEPData::PathResourceReference);

  m.def("make_CrossSectionMeasurement", &HEPData::make_CrossSectionMeasurement,
        py::arg("ref"), py::arg("local_cache_root") = ".")
      .def(
          "make_CrossSectionMeasurement",
          [](std::string const &ref,
             std::filesystem::path const &local_cache_root) {
            return make_CrossSectionMeasurement(HEPData::ResourceReference(ref),
                                                local_cache_root);
          },
          py::arg("ref"), py::arg("local_cache_root") = ".")
      .def("make_Record",
           py::overload_cast<HEPData::ResourceReference,
                             std::filesystem::path const &>(
               &HEPData::make_Record),
           py::arg("ref"), py::arg("local_cache_root") = ".")
      .def("make_Record",
           py::overload_cast<std::filesystem::path const &,
                             std::filesystem::path const &>(
               &HEPData::make_Record),
           py::arg("location"), py::arg("local_cache_root") = ".")
      .def("resolve_reference", &HEPData::resolve_reference, py::arg("ref"),
           py::arg("local_cache_root") = ".")
      .def(
          "resolve_reference",
          [](std::string const &ref,
             std::filesystem::path const &local_cache_root) {
            return HEPData::resolve_reference(HEPData::ResourceReference(ref),
                                              local_cache_root);
          },
          py::arg("ref"), py::arg("local_cache_root") = ".");
}