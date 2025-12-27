# ROS2 MPU6500 IMU Driver for Jetson Orin NX

This package provides a ROS2 driver for the MPU-6500 6-axis IMU sensor optimized for Jetson Orin NX.

## Features

- 3-axis accelerometer (±2g, ±4g, ±8g, ±16g ranges)
- 3-axis gyroscope (±250, ±500, ±1000, ±2000 °/s ranges)
- I2C communication interface
- Configurable sample rates up to 1 kHz
- Calibration offset support
- Standard `sensor_msgs/Imu` message output

## Hardware Setup

### MPU6500 Connections to Jetson Orin NX

| MPU6500 Pin | Jetson Orin NX | Description |
|-------------|----------------|-------------|
| VDD         | 3.3V           | Power supply |
| GND         | GND            | Ground |
| SCL         | I2C_GP5_CLK_3V3 (Pin 28) | I2C Clock |
| SDA         | I2C_GP5_DAT_3V3 (Pin 27) | I2C Data |
| AD0         | GND or 3.3V    | I2C address select (GND=0x68, 3.3V=0x69) |
| INT         | GPIO (optional)| Interrupt pin |

### Enable I2C on Jetson

```bash
# Check if I2C is enabled
ls /dev/i2c-*

# If not present, enable I2C
sudo modprobe i2c-dev

# Add user to i2c group
sudo usermod -aG i2c $USER

# Set I2C permissions (temporary, until reboot)
sudo chmod 666 /dev/i2c-1

# Or create udev rule for permanent permissions
echo 'KERNEL=="i2c-[0-9]*", GROUP="i2c", MODE="0660"' | sudo tee /etc/udev/rules.d/99-i2c.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### Verify MPU6500 Connection

```bash
# Install i2c-tools
sudo apt-get install i2c-tools

# Scan I2C bus (should show 0x68 or 0x69)
sudo i2cdetect -y -r 1
```

## Software Installation

### Prerequisites

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    libi2c-dev \
    i2c-tools \
    ros-${ROS_DISTRO}-sensor-msgs \
    ros-${ROS_DISTRO}-rclcpp
```

### Build

```bash
# Navigate to your ROS2 workspace
cd ~/ros2_ws/src

# Clone the repository
git clone <your-repo-url> ros2_mpu6500

# Build the package
cd ~/ros2_ws
colcon build --packages-select ros2_mpu6500

# Source the workspace
source install/setup.bash
```

## Configuration

Edit `config/params.yaml` to configure your sensor:

```yaml
mpu6500_sensor:
  ros__parameters:
    i2c_device: "/dev/i2c-1"      # I2C bus
    i2c_address: 0x68              # 0x68 or 0x69
    publish_rate_hz: 100           # 1-1000 Hz
    frame_id: "imu_link"
    
    # Calibration offsets (see Calibration section)
    gyro_x_offset: 0.0
    gyro_y_offset: 0.0
    gyro_z_offset: 0.0
    accel_x_offset: 0.0
    accel_y_offset: 0.0
    accel_z_offset: 0.0
```

## Usage

### Launch the Node

```bash
# Using launch file
ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py

# Or run directly
ros2 run ros2_mpu6500 ros2_mpu6500
```

### View IMU Data

```bash
# Echo IMU messages
ros2 topic echo /imu/data

# View topic info
ros2 topic info /imu/data

# Check publishing rate
ros2 topic hz /imu/data
```

### Visualize with RViz2

```bash
# Launch RViz2
rviz2

# Add:
# 1. Set Fixed Frame to "imu_link"
# 2. Add "Imu" display
# 3. Set topic to "/imu/data"
```

## Calibration

### Gyroscope Calibration

1. Place IMU on a stable, stationary surface
2. Record data for 30 seconds:

```bash
ros2 topic echo /imu/data > gyro_cal.txt
```

3. Calculate average angular velocities
4. Set negative values in `params.yaml`:

```yaml
gyro_x_offset: -0.0123  # Negative of average
gyro_y_offset: 0.0045
gyro_z_offset: -0.0067
```

### Accelerometer Calibration

1. Place IMU on a flat, level surface
2. Record data for 30 seconds
3. Calculate averages:
   - X and Y should be near 0
   - Z should be near 9.81 m/s²
4. Set offsets to achieve desired zero points

## Troubleshooting

### "Failed to open I2C device"
- Check I2C permissions: `ls -la /dev/i2c-1`
- Add user to i2c group: `sudo usermod -aG i2c $USER`
- Reboot or re-login

### "Failed to set I2C slave address"
- Verify MPU6500 is connected: `sudo i2cdetect -y -r 1`
- Check AD0 pin connection (determines address 0x68 or 0x69)
- Try the other address in config file

### "Failed to initialize MPU6500"
- Check power supply (3.3V stable)
- Verify all connections
- Check for shorts or loose wires
- Measure voltage at VDD pin (should be 3.3V ±5%)

### No Data Published
- Check if node is running: `ros2 node list`
- Verify topic exists: `ros2 topic list`
- Check for errors: `ros2 node info /mpu6500_sensor`

### Noisy Data
- Increase filtering in MPU6500 (modify code to use lower DLPF setting)
- Reduce publish rate
- Check for electrical interference
- Ensure stable power supply

## MPU6500 Specifications

From the datasheet:

- **Gyroscope Range**: ±250, ±500, ±1000, ±2000 °/s
- **Accelerometer Range**: ±2g, ±4g, ±8g, ±16g
- **Gyro Noise**: 0.01 °/s/√Hz
- **Accel Noise**: 300 μg/√Hz
- **Sample Rate**: Up to 8 kHz (1 kHz typical)
- **Operating Voltage**: 1.71V - 3.45V (3.3V recommended)
- **Operating Current**: 3.4 mA (all axes active)
- **Operating Temperature**: -40°C to +85°C

## ROS2 Message Format

The node publishes `sensor_msgs/msg/Imu` messages with:

```
Header header
  stamp: Current time
  frame_id: Configured frame ID (default: "imu_link")

Quaternion orientation: [0, 0, 0, 0] (not calculated)
  orientation_covariance[0]: -1 (indicates unknown)

Vector3 angular_velocity: [x, y, z] in rad/s
  angular_velocity_covariance: [0.0001, 0, 0, 0, 0.0001, 0, 0, 0, 0.0001]

Vector3 linear_acceleration: [x, y, z] in m/s²
  linear_acceleration_covariance: [0.01, 0, 0, 0, 0.01, 0, 0, 0, 0.01]
```

## License

MIT License

## References

- [MPU-6500 Product Specification](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6500/)
- [Jetson Orin NX Developer Kit](https://developer.nvidia.com/embedded/jetson-orin-nx-devkit)
- [ROS2 sensor_msgs/Imu](http://docs.ros.org/en/rolling/p/sensor_msgs/interfaces/msg/Imu.html)
