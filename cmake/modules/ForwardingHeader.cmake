function ( create_forwarding_header header )

  message ( STATUS "Creating forwarding header ${header} from directories ${ARGN}." )
  
  set ( files "" )  
  foreach ( d ${ARGN} )
    #message ( STATUS "Reading all files from ${d}" )
    file ( GLOB tmpfiles RELATIVE "${CMAKE_SOURCE_DIR}/include" "${d}/*.hpp" )
    list ( APPEND files  ${tmpfiles} )
    
    file ( GLOB generated_include_files "${GENERATED_INCLUDES_DIR}/${d}/*.hpp" )
    foreach ( generated_file_abspath ${generated_include_files} )
       file ( RELATIVE_PATH generated_file_relpath "${GENERATED_INCLUDES_DIR}/include/" ${generated_file_abspath} )
       list ( APPEND files ${generated_file_relpath} )  
    endforeach ()
  endforeach ()
  
  list ( SORT files )
  
  set ( GPCXX_INCLUDE_LIST "" )
  foreach ( f ${files} )
    set ( GPCXX_INCLUDE_LIST "${GPCXX_INCLUDE_LIST}\n#include <${f}>" )
    #message ( STATUS "FILE ${f}" )
  endforeach ()
  
  # message ( STATUS "${GPCXX_INCLUDE_LIST}" )
  
  string ( SUBSTRING ${header} 8 -1 GPCXX_FILE_NAME )
  
  string ( TOUPPER ${header} header_f )
  string ( SUBSTRING ${header_f} 8 -1 header_f )
  string ( REPLACE "/" "_" header_f ${header_f} )
  string ( REPLACE "." "_" header_f ${header_f} )
  set ( GPCXX_HEADER_GUARD "${header_f}_DEFINED" )
    
  configure_file ( "${CMAKE_SOURCE_DIR}/include/gpcxx/forwarding_header.hpp.cmake" "${GENERATED_INCLUDES_DIR}/${header}" )


endfunction ()
