cmake_minimum_required(VERSION 3.8)
project(ros2_mpu6500)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

# Include directories
include_directories(include)

# libdriver MPU6500 paths
set(MPU6500_DRIVER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../mpu6500")
include_directories(${MPU6500_DRIVER_DIR}/src)
include_directories(${MPU6500_DRIVER_DIR}/interface)

# Enable C language
enable_language(C)

# libdriver MPU6500 source files
set(MPU6500_DRIVER_SOURCES
  ${MPU6500_DRIVER_DIR}/src/driver_mpu6500.c
)

# Add executable with all sources
add_executable(ros2_mpu6500 
  src/mpu6500_interface.c
  src/mpu6500.cpp 
  src/mpu6500_hal.cpp 
  src/mpu6500_node.cpp
  ${MPU6500_DRIVER_SOURCES}
)

ament_target_dependencies(ros2_mpu6500 rclcpp sensor_msgs)

# Link I2C library
target_link_libraries(ros2_mpu6500 i2c)

# Install targets
install(TARGETS
  ros2_mpu6500
  DESTINATION lib/${PROJECT_NAME}
)

# Install directories
install(
  DIRECTORY include config launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()