cmake_minimum_required(VERSION 2.8.3)
project(orion_protocol)

add_definitions(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    rospy
)

catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS roscpp
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

set(MAJOR_FILES
  src/major/orion_major.cpp
)

set(MAJOR_UTILS_FILES
  src/major/orion_assert/ros_assert.cpp
  src/common/orion_memory/heap_memory.c
  src/common/orion_timeout.c
  src/common/orion_circular_buffer.c
)

set(TRANSPORT_FRAMED_FILES
  src/common/orion_framer/cobs_framer.c
  src/common/orion_crc.c
  src/common/orion_transport/frame_transport.c
)

set(COMMUNICATION_SERIAL_FILES
  src/major/orion_communication/serial_port.c
)

set(COMMUNICATION_TCP_BRIDGE_FILES
  src/major/orion_communication/tcp_serial_bridge.c
)

set(MINOR_FILES
  src/minor/orion_minor.c
)

add_library(${PROJECT_NAME}
  ${MAJOR_FILES}
  ${MAJOR_UTILS_FILES}
  ${TRANSPORT_FRAMED_FILES}
  ${COMMUNICATION_SERIAL_FILES}
)

add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

target_link_libraries(${PROJECT_NAME}
  ${catkin_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".svn" EXCLUDE
)

if (CATKIN_ENABLE_TESTING)

  include_directories(
    test/include
    include
    ${catkin_INCLUDE_DIRS}
  )

  catkin_add_gmock(${PROJECT_NAME}_test_major test/test_orion_major.cpp)
  add_dependencies(${PROJECT_NAME}_test_major ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_major ${PROJECT_NAME})

  catkin_add_gmock(${PROJECT_NAME}_test_minor
    test/test_orion_minor.cpp
    ${MINOR_FILES}
    ${MAJOR_UTILS_FILES}
    ${MAJOR_FILES})
  add_dependencies(${PROJECT_NAME}_test_minor ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_minor ${catkin_LIBRARIES})

  catkin_add_gmock(${PROJECT_NAME}_test_cobs_framer test/test_orion_cobs_framer.cpp)
  add_dependencies(${PROJECT_NAME}_test_cobs_framer ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_cobs_framer ${PROJECT_NAME})

  catkin_add_gmock(${PROJECT_NAME}_test_frame_transport test/test_orion_frame_transport.cpp)
  add_dependencies(${PROJECT_NAME}_test_frame_transport ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_frame_transport ${PROJECT_NAME})

  catkin_add_gmock(${PROJECT_NAME}_test_circular_buffer test/test_orion_circular_buffer.cpp)
  add_dependencies(${PROJECT_NAME}_test_circular_buffer ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_circular_buffer ${PROJECT_NAME})

  find_package(rostest REQUIRED)
  add_rostest_gmock(test_tcp_bridge_integration 
    test/test_tcp_bridge_integration.test
    test/test_tcp_bridge_integration.cpp
    ${MAJOR_UTILS_FILES}
    ${MAJOR_FILES}
    ${TRANSPORT_FRAMED_FILES}
    ${COMMUNICATION_TCP_BRIDGE_FILES})
  add_dependencies(test_tcp_bridge_integration ${catkin_EXPORTED_TARGETS})
  target_link_libraries(test_tcp_bridge_integration ${catkin_LIBRARIES})

endif ()
