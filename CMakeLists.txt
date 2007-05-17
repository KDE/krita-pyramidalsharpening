include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/lib)

set(kritaPyramidalSharpening_PART_SRCS PyramidalSharpening.cc  )

kde3_automoc(${kritaPyramidalSharpening_PART_SRCS})

kde3_add_kpart(kritaPyramidalSharpening ${kritaPyramidalSharpening_PART_SRCS})

target_link_libraries(kritaPyramidalSharpening ${KRITAUI_LIBRARY} kritapluginscommon)

install(TARGETS kritaPyramidalSharpening  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  kritaPyramidalSharpening.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
