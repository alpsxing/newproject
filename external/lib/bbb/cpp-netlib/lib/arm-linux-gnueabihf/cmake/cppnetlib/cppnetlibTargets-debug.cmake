#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cppnetlib-uri" for configuration "Debug"
set_property(TARGET cppnetlib-uri APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(cppnetlib-uri PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "/home/xingqq/newproject/external/lib/bbb/cpp-netlib/lib/arm-linux-gnueabihf/libcppnetlib-uri.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS cppnetlib-uri )
list(APPEND _IMPORT_CHECK_FILES_FOR_cppnetlib-uri "/home/xingqq/newproject/external/lib/bbb/cpp-netlib/lib/arm-linux-gnueabihf/libcppnetlib-uri.a" )

# Import target "cppnetlib-server-parsers" for configuration "Debug"
set_property(TARGET cppnetlib-server-parsers APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(cppnetlib-server-parsers PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "/home/xingqq/newproject/external/lib/bbb/cpp-netlib/lib/arm-linux-gnueabihf/libcppnetlib-server-parsers.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS cppnetlib-server-parsers )
list(APPEND _IMPORT_CHECK_FILES_FOR_cppnetlib-server-parsers "/home/xingqq/newproject/external/lib/bbb/cpp-netlib/lib/arm-linux-gnueabihf/libcppnetlib-server-parsers.a" )

# Import target "cppnetlib-client-connections" for configuration "Debug"
set_property(TARGET cppnetlib-client-connections APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(cppnetlib-client-connections PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "/home/xingqq/newproject/external/lib/bbb/openssl/lib/libssl.a;/home/xingqq/newproject/external/lib/bbb/openssl/lib/libcrypto.a;/home/xingqq/newproject/external/lib/bbb/boost/lib/libboost_unit_test_framework.a;/home/xingqq/newproject/external/lib/bbb/boost/lib/libboost_system.a;/home/xingqq/newproject/external/lib/bbb/boost/lib/libboost_regex.a;/home/xingqq/newproject/external/lib/bbb/boost/lib/libboost_date_time.a;/home/xingqq/newproject/external/lib/bbb/boost/lib/libboost_thread.a;/home/xingqq/newproject/external/lib/bbb/boost/lib/libboost_filesystem.a;/home/xingqq/newproject/external/lib/bbb/boost/lib/libboost_program_options.a;/home/xingqq/newproject/external/lib/bbb/boost/lib/libboost_chrono.a;/home/xingqq/newproject/external/lib/bbb/boost/lib/libboost_atomic.a"
  IMPORTED_LOCATION_DEBUG "/home/xingqq/newproject/external/lib/bbb/cpp-netlib/lib/arm-linux-gnueabihf/libcppnetlib-client-connections.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS cppnetlib-client-connections )
list(APPEND _IMPORT_CHECK_FILES_FOR_cppnetlib-client-connections "/home/xingqq/newproject/external/lib/bbb/cpp-netlib/lib/arm-linux-gnueabihf/libcppnetlib-client-connections.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
