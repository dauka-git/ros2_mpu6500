cmake_minimum_required(VERSION 3.8)
project(ros2_mpu6500)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../mpu6500/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../mpu6500/interface)

# Enable C language
enable_language(C)

# Add MPU6500 C sources
set(MPU6500_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/../mpu6500/src/driver_mpu6500.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mpu6500_interface.c
)

add_executable(ros2_mpu6500 src/mpu6500.cpp src/mpu6500_hal.cpp src/mpu6500_node.cpp ${MPU6500_SOURCES})
ament_target_dependencies(ros2_mpu6500 rclcpp sensor_msgs)
target_link_libraries(ros2_mpu6500 i2c)

install(
  DIRECTORY include
  DESTINATION include
)

install(TARGETS
  ros2_mpu6500
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()