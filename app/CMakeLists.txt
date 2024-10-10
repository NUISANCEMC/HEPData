add_executable(nuis-hepdata nuis-hepdata.cxx)
target_link_libraries(nuis-hepdata PRIVATE NUISANCEHEPData::All 
  docopt::docopt spdlog::spdlog yaml-cpp::yaml-cpp)

install(TARGETS nuis-hepdata DESTINATION bin)