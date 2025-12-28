#include "ros2_mpu6500/mpu6500_node.h"

#include <chrono>
#include <memory>
#include <cmath>

using namespace std::chrono_literals;

Mpu6500Node::Mpu6500Node(const std::string& name)
    : Node(name)
    , mpu6500_dev_{std::make_unique<Mpu6500>()}
{
    // Declare parameters
    this->declare_parameter<double>("gyro_x_offset", 0.0);
    this->declare_parameter<double>("gyro_y_offset", 0.0);
    this->declare_parameter<double>("gyro_z_offset", 0.0);
    this->declare_parameter<double>("accel_x_offset", 0.0);
    this->declare_parameter<double>("accel_y_offset", 0.0);
    this->declare_parameter<double>("accel_z_offset", 0.0);

    /* Assign offset values */
    gyro_x_offset_ = this->get_parameter("gyro_x_offset").as_double();
    gyro_y_offset_ = this->get_parameter("gyro_y_offset").as_double();
    gyro_z_offset_ = this->get_parameter("gyro_z_offset").as_double();
    accel_x_offset_ = this->get_parameter("accel_x_offset").as_double();
    accel_y_offset_ = this->get_parameter("accel_y_offset").as_double();
    accel_z_offset_ = this->get_parameter("accel_z_offset").as_double();

    publisher_ = this->create_publisher<sensor_msgs::msg::Imu>("imu/mpu6500", 10);

    timer_ = this->create_wall_timer(10ms, std::bind(&Mpu6500Node::ImuPubCallback, this));
}

void Mpu6500Node::ImuPubCallback()
{
    const double GRAVITY = 9.80665;
    const double DEG_TO_RAD = M_PI / 180.0;

    auto message = sensor_msgs::msg::Imu();
    message.header.stamp = this->get_clock()->now();
    message.header.frame_id = "base_link";
    message.linear_acceleration_covariance = {0};

    /* Read IMU data */
    Mpu6500::Mpu6500_AccelData_t AccelData;
    Mpu6500::Mpu6500_GyroData_t GyroData;

    if (mpu6500_dev_->Mpu6500_GetAccelData(AccelData) != Mpu6500Hal::MPU6500_OK) {
        RCLCPP_ERROR(this->get_logger(), "Failed to read accelerometer data");
        return;
    }

    if (mpu6500_dev_->Mpu6500_GetGyroData(GyroData) != Mpu6500Hal::MPU6500_OK) {
        RCLCPP_ERROR(this->get_logger(), "Failed to read gyroscope data");
        return;
    }

    message.linear_acceleration.x = (AccelData.Accel_X - accel_x_offset_) * GRAVITY;
    message.linear_acceleration.y = (AccelData.Accel_Y - accel_y_offset_) * GRAVITY;
    message.linear_acceleration.z = (AccelData.Accel_Z - accel_z_offset_) * GRAVITY;
    message.angular_velocity_covariance[0] = {0};
    message.angular_velocity.x = (GyroData.Gyro_X - gyro_x_offset_) * DEG_TO_RAD;
    message.angular_velocity.y = (GyroData.Gyro_Y - gyro_y_offset_) * DEG_TO_RAD;
    message.angular_velocity.z = (GyroData.Gyro_Z - gyro_z_offset_) * DEG_TO_RAD;

    RCLCPP_INFO(this->get_logger(), "Accel: x=%.3f, y=%.3f, z=%.3f | Gyro: x=%.3f, y=%.3f, z=%.3f",
                AccelData.Accel_X, AccelData.Accel_Y, AccelData.Accel_Z,
                GyroData.Gyro_X, GyroData.Gyro_Y, GyroData.Gyro_Z);

    message.orientation_covariance[0] = -1;
    message.orientation.x = 0;
    message.orientation.y = 0;
    message.orientation.z = 0;
    message.orientation.w = 0;
    publisher_->publish(message);
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Mpu6500Node>("mpu6500publisher");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}