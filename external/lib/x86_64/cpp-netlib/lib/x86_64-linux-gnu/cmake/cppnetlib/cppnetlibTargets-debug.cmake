#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cppnetlib-uri" for configuration "Debug"
set_property(TARGET cppnetlib-uri APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(cppnetlib-uri PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "/home/xingqq/Documents/new2/external/lib/cpp-netlib/lib/x86_64-linux-gnu/libcppnetlib-uri.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS cppnetlib-uri )
list(APPEND _IMPORT_CHECK_FILES_FOR_cppnetlib-uri "/home/xingqq/Documents/new2/external/lib/cpp-netlib/lib/x86_64-linux-gnu/libcppnetlib-uri.a" )

# Import target "cppnetlib-server-parsers" for configuration "Debug"
set_property(TARGET cppnetlib-server-parsers APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(cppnetlib-server-parsers PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "/home/xingqq/Documents/new2/external/lib/cpp-netlib/lib/x86_64-linux-gnu/libcppnetlib-server-parsers.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS cppnetlib-server-parsers )
list(APPEND _IMPORT_CHECK_FILES_FOR_cppnetlib-server-parsers "/home/xingqq/Documents/new2/external/lib/cpp-netlib/lib/x86_64-linux-gnu/libcppnetlib-server-parsers.a" )

# Import target "cppnetlib-client-connections" for configuration "Debug"
set_property(TARGET cppnetlib-client-connections APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(cppnetlib-client-connections PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "/home/xingqq/Documents/new2/external/lib/openssl/lib/libssl.so;/home/xingqq/Documents/new2/external/lib/openssl/lib/libcrypto.so;/home/xingqq/Documents/new2/external/lib/boost/lib/libboost_unit_test_framework.a;/home/xingqq/Documents/new2/external/lib/boost/lib/libboost_system.a;/home/xingqq/Documents/new2/external/lib/boost/lib/libboost_regex.a;/home/xingqq/Documents/new2/external/lib/boost/lib/libboost_date_time.a;/home/xingqq/Documents/new2/external/lib/boost/lib/libboost_thread.a;/home/xingqq/Documents/new2/external/lib/boost/lib/libboost_filesystem.a;/home/xingqq/Documents/new2/external/lib/boost/lib/libboost_program_options.a;/home/xingqq/Documents/new2/external/lib/boost/lib/libboost_chrono.a;/home/xingqq/Documents/new2/external/lib/boost/lib/libboost_atomic.a;/usr/lib/x86_64-linux-gnu/libpthread.so"
  IMPORTED_LOCATION_DEBUG "/home/xingqq/Documents/new2/external/lib/cpp-netlib/lib/x86_64-linux-gnu/libcppnetlib-client-connections.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS cppnetlib-client-connections )
list(APPEND _IMPORT_CHECK_FILES_FOR_cppnetlib-client-connections "/home/xingqq/Documents/new2/external/lib/cpp-netlib/lib/x86_64-linux-gnu/libcppnetlib-client-connections.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
