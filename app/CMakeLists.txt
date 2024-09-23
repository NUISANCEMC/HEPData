add_executable(nuis-hepdata nuis-hepdata.cxx)
target_link_libraries(nuis-hepdata PRIVATE NUISANCEHEPData::All docopt::docopt spdlog::spdlog)

install(TARGETS nuis-hepdata DESTINATION bin)