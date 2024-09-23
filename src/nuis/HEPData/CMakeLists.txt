set(HEADERS 
  ResourceReference.hxx
  ReferenceResolver.hxx)

set(IMPLEMENTATION 
  ResourceReference.cxx
  ReferenceResolver.cxx
  HEPDataTableFactory.cxx)

add_library(NUISANCEHEPData SHARED ${IMPLEMENTATION})
target_link_libraries(NUISANCEHEPData PUBLIC NUISANCEHEPData::Options yaml-cpp::yaml-cpp)
target_link_libraries(NUISANCEHEPData PRIVATE nuishpd_private_compile_options 
  cpr::cpr fmt::fmt-header-only spdlog::spdlog)
set_target_properties(NUISANCEHEPData PROPERTIES 
  PUBLIC_HEADER "${HEADERS}"
  EXPORT_NAME All)

install(TARGETS NUISANCEHEPData
    EXPORT NUISANCEHEPData-targets
    LIBRARY DESTINATION lib/
    PUBLIC_HEADER DESTINATION include/nuis/HEPData)

add_library(NUISANCEHEPData::All ALIAS NUISANCEHEPData)