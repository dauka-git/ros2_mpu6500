#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
import numpy as np
import sys

class IMUCalibration(Node):
    def init(self):
        super().init('imu_calibration')
        self.subscription = self.create_subscription(
            Imu,
            '/imu/data',
            self.imu_callback,
            10)
        
        self.accel_x = []
        self.accel_y = []
        self.accel_z = []
        self.gyro_x = []
        self.gyro_y = []
        self.gyro_z = []
        
        self.sample_count = 0
        self.target_samples = 500  # 5 seconds at 100Hz
        
        print("\n" + "="*60)
        print("MPU6500 IMU CALIBRATION")
        print("="*60)
        print(f"\nCollecting {self.target_samples} samples...")
        print("Keep the IMU completely still on a flat, level surface!")
        print("="*60 + "\n")

    def imu_callback(self, msg):
        if self.sample_count >= self.target_samples:
            return
            
        self.accel_x.append(msg.linear_acceleration.x)
        self.accel_y.append(msg.linear_acceleration.y)
        self.accel_z.append(msg.linear_acceleration.z)
        self.gyro_x.append(msg.angular_velocity.x)
        self.gyro_y.append(msg.angular_velocity.y)
        self.gyro_z.append(msg.angular_velocity.z)
        
        self.sample_count += 1
        
        if self.sample_count % 50 == 0:
            print(f"Progress: {self.sample_count}/{self.target_samples} samples")
        
        if self.sample_count == self.target_samples:
            self.calculate_offsets()
            rclpy.shutdown()
    
    def calculate_offsets(self):
        print("\n" + "="*60)
        print("CALIBRATION RESULTS")
        print("="*60)
        
        # Calculate means
        accel_x_mean = np.mean(self.accel_x)
        accel_y_mean = np.mean(self.accel_y)
        accel_z_mean = np.mean(self.accel_z)
        gyro_x_mean = np.mean(self.gyro_x)
        gyro_y_mean = np.mean(self.gyro_y)
        gyro_z_mean = np.mean(self.gyro_z)
        
        # Calculate standard deviations
        accel_x_std = np.std(self.accel_x)
        accel_y_std = np.std(self.accel_y)
        accel_z_std = np.std(self.accel_z)
        gyro_x_std = np.std(self.gyro_x)
        gyro_y_std = np.std(self.gyro_y)
        gyro_z_std = np.std(self.gyro_z)
        
        print("\nCurrent Readings (mean ± std):")
        print(f"  Accel X: {accel_x_mean:8.4f} ± {accel_x_std:.4f} m/s²")
        print(f"  Accel Y: {accel_y_mean:8.4f} ± {accel_y_std:.4f} m/s²")
        print(f"  Accel Z: {accel_z_mean:8.4f} ± {accel_z_std:.4f} m/s²")
        print(f"  Gyro  X: {gyro_x_mean:8.6f} ± {gyro_x_std:.6f} rad/s")
        print(f"  Gyro  Y: {gyro_y_mean:8.6f} ± {gyro_y_std:.6f} rad/s")
        print(f"  Gyro  Z: {gyro_z_mean:8.6f} ± {gyro_z_std:.6f} rad/s")
        
        # Calculate magnitude
        accel_mag = np.sqrt(accel_x_mean**2 + accel_y_mean**2 + accel_z_mean**2)
        print(f"\nAcceleration Magnitude: {accel_mag:.4f} m/s² (should be ~9.81)")
        
        # Calculate offsets (what to subtract to get desired values)
        # For accelerometer: X and Y should be 0, Z should be +9.81 (or -9.81 if upside down)
        accel_x_offset = accel_x_mean - 0.0
        accel_y_offset = accel_y_mean - 0.0
        
        # Determine if Z should be positive or negative
        if abs(accel_z_mean - 9.81) < abs(accel_z_mean + 9.81):
            # Z is closer to +9.81 (right-side up)
            accel_z_offset = accel_z_mean - 9.81
            orientation = "RIGHT-SIDE UP"
        else:
            # Z is closer to -9.81 (upside down)
            accel_z_offset = accel_z_mean + 9.81
            orientation = "UPSIDE DOWN"
        
        # For gyroscope: all should be 0
        gyro_x_offset = gyro_x_mean
        gyro_y_offset = gyro_y_mean
        gyro_z_offset = gyro_z_mean
        
        print(f"\nIMU Orientation: {orientation}")
        print("\n" + "="*60)
        print("COPY THESE VALUES TO config/params.yaml:")
        print("="*60)
        print(f"""
mpu6500_sensor:
  ros__parameters:
    i2c_device: "/dev/i2c-7"
    i2c_address: 0x68
    publish_rate_hz: 100
    frame_id: "imu_link"
    
    # Gyroscope offsets (rad/s)
    gyro_x_offset: {gyro_x_offset:.6f}
    gyro_y_offset: {gyro_y_offset:.6f}
    gyro_z_offset: {gyro_z_offset:.6f}
    
    # Accelerometer offsets (m/s²)
    accel_x_offset: {accel_x_offset:.6f}
    accel_y_offset: {accel_y_offset:.6f}
    accel_z_offset: {accel_z_offset:.6f}
""")
        print("="*60)
        print("\nAfter updating the config file, rebuild and relaunch:")
        print("  cd ~/ros2_ws")
        print("  colcon build --packages-select ros2_mpu6500")
        print("  source install/setup.bash")
        print("  ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py")
        print("="*60 + "\n")

def main(args=None):
    rclpy.init(args=args)
    calibration = IMUCalibration()
    
    try:
        rclpy.spin(calibration)
    except KeyboardInterrupt:
        print("\nCalibration interrupted!")
    finally:
        calibration.destroy_node()

if __name__ == '__main__':
    main()