pybind11_add_module(pyNUISANCEHEPData SHARED 
  pyNUISANCEHEPData.cxx
)

target_link_libraries(pyNUISANCEHEPData 
  PRIVATE 
    NUISANCEHEPData
    spdlog::spdlog
    pybind11::module)

target_include_directories(pyNUISANCEHEPData PRIVATE
  ${Python3_INCLUDE_DIRS})

install(TARGETS pyNUISANCEHEPData
    LIBRARY DESTINATION ${NUISANCEHEPData_PYTHONPATH})