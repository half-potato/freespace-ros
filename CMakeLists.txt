cmake_minimum_required(VERSION 2.8.11)
project(freespace_ros)
#set(CMAKE_CXX_COMPILER "g++")
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_CXX_FLAGS -std=c++0x)

find_package(catkin REQUIRED COMPONENTS
	rospy
	roscpp
	cv_bridge
	image_transport
)
include_directories(${catkin_INCLUDE_DIRS})

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES freespace_ros
#  CATKIN_DEPENDS rospy
#  DEPENDS system_lib
)

#find_package(ROOT REQUIRED COMPONENTS RIO Net)
#include(${ROOT_USE_FILE})
#include_directories(${ROOT_INCLUDE_DIRS})

add_definitions(
	-Wall
	#-std=c++11
	#-std=c++0x
	#-O0
	#-g
)

#########
## ROS ##
#########


find_package(roscpp REQUIRED)
include_directories(${roscpp_INCLUDE_DIRS})
find_package(libfreespace REQUIRED)
include_directories(${LIBFREESPACE_INCLUDE_DIRS})
# add_message_files(
#   FILES
# )
# generate_messages(
#   DEPENDENCIES
#   std_msgs  # Or other packages containing msgs
# )

set(ALL_LIBS
	${CATKIN_LIBRARIES}
	${roscpp_LIBRARIES}
	${LIBFREESPACE_LIBRARIES}
)

add_executable(freespace_ros src/main.cpp)
target_link_libraries(freespace_ros ${ALL_LIBS})
