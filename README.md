| Service  | Status |
|---------------|--------|
| Code Style Checks | [![CircleCI](https://circleci.com/gh/ros-ukraine/orion.svg?style=shield)](https://circleci.com/gh/ros-ukraine/orion) |
| Unit and Integration Tests | [![Build Status](https://api.travis-ci.org/ros-ukraine/orion.svg?branch=kinetic-devel)](https://travis-ci.org/github/ros-ukraine/orion) |

# Orion
Protocol for communication between robot's computer and MCU

[![License](https://img.shields.io/github/license/ros-ukraine/orion.svg)](https://github.com/ros-ukraine/orion/blob/kinetic-devel/LICENSE)

## Development

* Create ROS workspace using these instructions http://wiki.ros.org/catkin/Tutorials/create_a_workspace
* Go to src folder of workspace which you have created
* Clone this repository
```bash
git clone https://github.com/ros-ukraine/orion.git
```
* Go to workspace directory
```bash
cd ..
```
* Compilation
```bash
catkin_make
```
* Unit tests compilation and execution
```bash
catkin_make run_tests
```
