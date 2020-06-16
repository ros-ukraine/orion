cmake_minimum_required(VERSION 2.8.3)
project(orion_protocol)

add_definitions(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
    roscpp
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
#  src/orion_framer.cpp
  src/orion_timeout.cpp
  src/orion_com_transport.cpp
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


  catkin_add_gmock(${PROJECT_NAME}_test_protocol test/test_orion_protocol.cpp)
  add_dependencies(${PROJECT_NAME}_test_protocol ${catkin_EXPORTED_TARGETS})
  target_link_libraries(${PROJECT_NAME}_test_protocol ${PROJECT_NAME})
endif ()
