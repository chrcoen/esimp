set(SYSTEMC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../systemc")
add_library(systemc STATIC IMPORTED)
set_target_properties(systemc PROPERTIES
  IMPORTED_LOCATION "${SYSTEMC_DIR}/lib-linux64-debug/libsystemc.a"
  INTERFACE_INCLUDE_DIRECTORIES "${SYSTEMC_DIR}/include"
)
