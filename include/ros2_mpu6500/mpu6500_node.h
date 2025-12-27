#ifndef MPU6500_NODE_H
#define MPU6500_NODE_H

#include "ros2_mpu6500/mpu6500.h"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"

class Mpu6500Node : public rclcpp::Node {
 public:
  Mpu6500Node(const std::string& name);

 private:
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr publisher_;
  std::unique_ptr<Mpu6500> mpu6500_dev_;
  rclcpp::TimerBase::SharedPtr timer_;
  void ImuPubCallback();

  double gyro_x_offset_ {0.0};
  double gyro_y_offset_ {0.0};
  double gyro_z_offset_ {0.0};
  double accel_x_offset_ {0.0};
  double accel_y_offset_ {0.0};
  double accel_z_offset_ {0.0};

};

#endif  // MPU6500_NODE_H