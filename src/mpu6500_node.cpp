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

    RCLCPP_INFO(this->get_logger(), "IMU Calibration Offsets:");
    RCLCPP_INFO(this->get_logger(), "  Gyro: [%.4f, %.4f, %.4f] rad/s", 
                gyro_x_offset_, gyro_y_offset_, gyro_z_offset_);
    RCLCPP_INFO(this->get_logger(), "  Accel: [%.4f, %.4f, %.4f] m/s²", 
                accel_x_offset_, accel_y_offset_, accel_z_offset_);

    publisher_ = this->create_publisher<sensor_msgs::msg::Imu>("imu/data", 10);

    timer_ = this->create_wall_timer(10ms, std::bind(&Mpu6500Node::ImuPubCallback, this));
}

void Mpu6500Node::ImuPubCallback()
{
    auto message = sensor_msgs::msg::Imu();
    message.header.stamp = this->get_clock()->now();
    message.header.frame_id = "imu_link";

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

    // Data is already in m/s² and rad/s, just apply offsets (NO double conversion!)
    message.linear_acceleration.x = AccelData.Accel_X - accel_x_offset_;
    message.linear_acceleration.y = AccelData.Accel_Y - accel_y_offset_;
    message.linear_acceleration.z = AccelData.Accel_Z - accel_z_offset_;
    
    message.angular_velocity.x = GyroData.Gyro_X - gyro_x_offset_;
    message.angular_velocity.y = GyroData.Gyro_Y - gyro_y_offset_;
    message.angular_velocity.z = GyroData.Gyro_Z - gyro_z_offset_;

    // Set covariance matrices (diagonal values)
    // Angular velocity covariance (based on gyro noise: 0.01 °/s/√Hz)
    message.angular_velocity_covariance[0] = 0.0001;  // x variance
    message.angular_velocity_covariance[4] = 0.0001;  // y variance
    message.angular_velocity_covariance[8] = 0.0001;  // z variance
    
    // Linear acceleration covariance (based on accel noise: 300 μg/√Hz)
    message.linear_acceleration_covariance[0] = 0.01;  // x variance
    message.linear_acceleration_covariance[4] = 0.01;  // y variance
    message.linear_acceleration_covariance[8] = 0.01;  // z variance

    // Orientation is not calculated (no magnetometer)
    message.orientation_covariance[0] = -1.0;  // Indicates unknown orientation
    message.orientation.x = 0.0;
    message.orientation.y = 0.0;
    message.orientation.z = 0.0;
    message.orientation.w = 0.0;

    publisher_->publish(message);
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Mpu6500Node>("mpu6500_sensor");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}