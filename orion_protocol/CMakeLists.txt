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
  src/orion_serial_port.cpp
  src/orion_tcp_serial_bridge.cpp
  src/orion_framer.cpp
  src/orion_cobs_framer.cpp
  src/orion_crc.cpp
  src/orion_frame_transport.cpp
  src/orion_timeout.cpp
  src/orion_major.cpp
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


  catkin_add_gmock(${PROJECT_NAME}_test_framer test/test_orion_framer.cpp)
  add_dependencies(${PROJECT_NAME}_test_framer ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_framer ${PROJECT_NAME})

#!!!! TODO !!!
#  find_package(rostest REQUIRED)
#  add_rostest_gtest(tests_mynode test/mynode.test src/test/test_mynode.cpp [more cpp files])
#  target_link_libraries(tests_mynode ${catkin_LIBRARIES})

endif ()
