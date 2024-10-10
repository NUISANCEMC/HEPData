set(HEADERS 
  Tables.h
  Variables.h
  Record.h
  ResourceReference.h
  ReferenceResolver.h
  TableFactory.h
  StreamHelpers.h
  YAMLConverters.h
  CrossSectionMeasurement.h)

set(IMPLEMENTATION
  CrossSectionMeasurement.cxx
  ResourceReference.cxx
  ReferenceResolver.cxx
  TableFactory.cxx
  StreamHelpers.cxx
  YAMLConverters.cxx)

add_library(NUISANCEHEPData SHARED ${IMPLEMENTATION})
target_link_libraries(NUISANCEHEPData PUBLIC nuishpd_options)
target_link_libraries(NUISANCEHEPData PRIVATE nuishpd_private_compile_options 
  cpr::cpr spdlog::spdlog yaml-cpp::yaml-cpp)
set_target_properties(NUISANCEHEPData PROPERTIES 
  PUBLIC_HEADER "${HEADERS}"
  EXPORT_NAME All)

install(TARGETS NUISANCEHEPData
    EXPORT NUISANCEHEPData-targets
    LIBRARY DESTINATION lib/
    PUBLIC_HEADER DESTINATION include/nuis/HEPData)

add_library(NUISANCEHEPData::All ALIAS NUISANCEHEPData)