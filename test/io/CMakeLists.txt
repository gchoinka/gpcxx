# Date: 2013-01-25
# Author: Karsten Ahnert (karsten.ahnert@gmx.de)

include_directories ( ${gtest_SOURCE_DIR}/include )
include_directories ( ${gtest_SOURCE_DIR} )


add_executable ( io_tests
  bracket.cpp
  simple.cpp
  graphviz.cpp
  polish.cpp
  json.cpp
  population_json.cpp
  )

target_link_libraries ( io_tests gtest gtest_main )

add_test( NAME io_tests COMMAND io_tests )

# add_custom_command(TARGET UtilTests POST_BUILD COMMAND UtilTests )