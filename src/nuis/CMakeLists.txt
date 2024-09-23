set(HEADERS 
  ResourceReference.hxx
  ReferenceResolver.hxx)

set(IMPLEMENTATION 
  ResourceReference.cxx
  ReferenceResolver.cxx
  HEPDataTableFactory.cxx)

add_library(nuishpd SHARED ${IMPLEMENTATION})
target_link_libraries(nuishpd PUBLIC NUISANCEHEPData::Options yaml-cpp::yaml-cpp)
target_link_libraries(nuishpd PRIVATE nuishpd_private_compile_options cpr::cpr fmt::fmt-header-only spdlog::spdlog)
set_target_properties(nuishpd PROPERTIES 
  PUBLIC_HEADER "${HEADERS}"
  EXPORT_NAME All)

install(TARGETS nuishpd
    EXPORT nuishpd-targets
    LIBRARY DESTINATION lib/
    PUBLIC_HEADER DESTINATION include/NuHepMC)

add_library(NUISANCEHEPData::All ALIAS nuishpd)

add_executable(testapp testapp.cxx)
target_link_libraries(testapp NUISANCEHEPData::All)