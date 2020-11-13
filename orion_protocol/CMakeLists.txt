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

add_library(${PROJECT_NAME}
  src/major/orion_serial_port.cpp
  src/major/orion_tcp_serial_bridge.cpp
  src/common/orion_framer.cpp
  src/common/orion_cobs_framer.cpp
  src/common/orion_crc.cpp
  src/common/orion_frame_transport.cpp
  src/common/orion_timeout.cpp
  src/common/orion_circular_buffer.c
  src/major/orion_major.cpp
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
  catkin_add_gmock(${PROJECT_NAME}_test_major test/test_orion_major.cpp)
  add_dependencies(${PROJECT_NAME}_test_major ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_major ${PROJECT_NAME})

  catkin_add_gmock(${PROJECT_NAME}_test_minor test/test_orion_minor.cpp src/minor/orion_minor.cpp)
  add_dependencies(${PROJECT_NAME}_test_minor ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_minor ${PROJECT_NAME})

  catkin_add_gmock(${PROJECT_NAME}_test_cobs_framer test/test_orion_cobs_framer.cpp)
  add_dependencies(${PROJECT_NAME}_test_cobs_framer ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_cobs_framer ${PROJECT_NAME})

  catkin_add_gmock(${PROJECT_NAME}_test_frame_transport test/test_orion_frame_transport.cpp)
  add_dependencies(${PROJECT_NAME}_test_frame_transport ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_frame_transport ${PROJECT_NAME})

  find_package(rostest REQUIRED)
  add_rostest_gmock(test_tcp_bridge_integration test/test_tcp_bridge_integration.test test/test_tcp_bridge_integration.cpp)
  target_link_libraries(test_tcp_bridge_integration ${PROJECT_NAME})

endif ()
