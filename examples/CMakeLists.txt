# Create executables for each example
add_executable(example_mcap_writer example_mcap_writer.cpp)
add_executable(example_native_binary_reader example_native_binary_reader.cpp)
add_executable(example_txth_reader example_txth_reader.cpp)
add_executable(convert_osi2mcap convert_osi2mcap.cpp)

# Link against OSI made available by parent CMakeLists.txt
if(LINK_WITH_SHARED_OSI)
    target_link_libraries(example_mcap_writer PRIVATE open_simulation_interface)
    target_link_libraries(example_native_binary_reader PRIVATE open_simulation_interface)
    target_link_libraries(example_txth_reader PRIVATE open_simulation_interface)
    target_link_libraries(convert_osi2mcap PRIVATE open_simulation_interface)
else()
    target_link_libraries(example_mcap_writer PRIVATE open_simulation_interface_pic)
    target_link_libraries(example_native_binary_reader PRIVATE open_simulation_interface_pic)
    target_link_libraries(example_txth_reader PRIVATE open_simulation_interface_pic)
    target_link_libraries(convert_osi2mcap PRIVATE open_simulation_interface_pic)
    include_directories(${OSI_INCLUDE_DIR})
endif()

# Link each example against the library
target_link_libraries(example_mcap_writer PRIVATE OSIUtilities)
target_link_libraries(example_native_binary_reader PRIVATE OSIUtilities)
target_link_libraries(example_txth_reader PRIVATE OSIUtilities)
target_link_libraries(convert_osi2mcap PRIVATE OSIUtilities)

# Specify the public headers for the library for clean access
target_include_directories(example_mcap_writer PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(example_native_binary_reader PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(example_txth_reader PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(convert_osi2mcap PUBLIC ${PROJECT_SOURCE_DIR}/include)