# How to View Calibration Values from params.yaml

After calibration, you can view your calibration offsets in several ways:

## 1. **In Node Startup Logs** (Automatic Display)

When you start the node, it automatically prints the calibration values:

```bash
ros2 run ros2_mpu6500 ros2_mpu6500
# OR
ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py
```

**You'll see output like:**
```
[INFO] [mpu6500_sensor]: IMU Calibration Offsets:
[INFO] [mpu6500_sensor]:   Gyro: [0.0123, -0.0045, 0.0067] rad/s
[INFO] [mpu6500_sensor]:   Accel: [0.1234, -0.0567, -9.8100] m/s²
```

This is printed from `mpu6500_node.cpp` lines 29-33.

---

## 2. **Using ROS2 Parameter Commands**

### List all parameters:
```bash
ros2 param list /mpu6500_sensor
```

**Output:**
```
/mpu6500_sensor:
  accel_x_offset
  accel_y_offset
  accel_z_offset
  gyro_x_offset
  gyro_y_offset
  gyro_z_offset
  use_sim_time
```

### Get individual parameter values:
```bash
# Get gyroscope offsets
ros2 param get /mpu6500_sensor gyro_x_offset
ros2 param get /mpu6500_sensor gyro_y_offset
ros2 param get /mpu6500_sensor gyro_z_offset

# Get accelerometer offsets
ros2 param get /mpu6500_sensor accel_x_offset
ros2 param get /mpu6500_sensor accel_y_offset
ros2 param get /mpu6500_sensor accel_z_offset
```

**Example output:**
```
Double value is: 0.0123
```

### Get all calibration parameters at once:
```bash
ros2 param get /mpu6500_sensor gyro_x_offset gyro_y_offset gyro_z_offset accel_x_offset accel_y_offset accel_z_offset
```

---

## 3. **View the params.yaml File Directly**

Simply open the configuration file:

```bash
cat config/params.yaml
# OR
nano config/params.yaml
# OR open in your editor
```

**You'll see:**
```yaml
mpu6500_sensor:
  ros__parameters:
    i2c_device: "/dev/i2c-7"
    i2c_address: 0x68
    publish_rate_hz: 100
    frame_id: "imu_link"
    
    # Your calibrated offsets
    gyro_x_offset: 0.0123
    gyro_y_offset: -0.0045
    gyro_z_offset: 0.0067
    accel_x_offset: 0.1234
    accel_y_offset: -0.0567
    accel_z_offset: -9.8100
```

---

## 4. **Verify Offsets Are Applied** (In Published Messages)

The calibration offsets are **automatically applied** to all published IMU messages. You can verify this by:

### View raw IMU messages:
```bash
ros2 topic echo /imu/data
```

**Look at the `angular_velocity` and `linear_acceleration` fields:**
- These values have the offsets **already subtracted**
- When sensor is stationary, gyro values should be near **0 rad/s**
- When sensor is flat, accel Z should be near **0 m/s²** (or 9.81 m/s² if you calibrated for gravity)

**Example message:**
```yaml
angular_velocity:
  x: 0.0001    # Should be ~0 when stationary (offset applied)
  y: -0.0002
  z: 0.0001
linear_acceleration:
  x: 0.05      # Should be ~0 when flat (offset applied)
  y: -0.03
  z: 9.81      # Or 0 if you calibrated for zero-g
```

---

## 5. **Compare Before/After Calibration**

To see the effect of calibration:

### Before calibration (all offsets = 0.0):
```bash
# Edit params.yaml, set all offsets to 0.0
# Run node and record data
ros2 topic echo /imu/data > before_calibration.txt
```

### After calibration (with your offsets):
```bash
# Edit params.yaml with your calculated offsets
# Run node and record data
ros2 topic echo /imu/data > after_calibration.txt
```

### Compare:
```bash
# Check if gyro values are closer to zero
# Check if accel values match your expected values
```

---

## 6. **Add Debug Output to See Applied Values**

You can modify the code to print the values before and after applying offsets:

### Edit `src/mpu6500_node.cpp`:

Add this in `ImuPubCallback()` function (around line 59):

```cpp
void Mpu6500Node::ImuPubCallback()
{
    // ... existing code to read data ...
    
    // DEBUG: Print raw values before offset
    RCLCPP_DEBUG(this->get_logger(), 
                 "Raw - Accel: [%.4f, %.4f, %.4f] m/s², Gyro: [%.4f, %.4f, %.4f] rad/s",
                 AccelData.Accel_X, AccelData.Accel_Y, AccelData.Accel_Z,
                 GyroData.Gyro_X, GyroData.Gyro_Y, GyroData.Gyro_Z);
    
    // Apply offsets
    message.linear_acceleration.x = AccelData.Accel_X - accel_x_offset_;
    message.linear_acceleration.y = AccelData.Accel_Y - accel_y_offset_;
    message.linear_acceleration.z = AccelData.Accel_Z - accel_z_offset_;
    
    message.angular_velocity.x = GyroData.Gyro_X - gyro_x_offset_;
    message.angular_velocity.y = GyroData.Gyro_Y - gyro_y_offset_;
    message.angular_velocity.z = GyroData.Gyro_Z - gyro_z_offset_;
    
    // DEBUG: Print calibrated values after offset
    RCLCPP_DEBUG(this->get_logger(), 
                 "Calibrated - Accel: [%.4f, %.4f, %.4f] m/s², Gyro: [%.4f, %.4f, %.4f] rad/s",
                 message.linear_acceleration.x, message.linear_acceleration.y, 
                 message.linear_acceleration.z,
                 message.angular_velocity.x, message.angular_velocity.y, 
                 message.angular_velocity.z);
    
    // ... rest of code ...
}
```

Then enable debug logging:
```bash
ros2 run ros2_mpu6500 ros2_mpu6500 --ros-args --log-level debug
```

---

## Quick Reference Commands

```bash
# 1. View in startup logs
ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py

# 2. View via ROS2 parameters (while node is running)
ros2 param get /mpu6500_sensor gyro_x_offset

# 3. View config file
cat config/params.yaml

# 4. View applied values in messages
ros2 topic echo /imu/data

# 5. Check if values are correct (should be ~0 when stationary)
ros2 topic echo /imu/data | grep -A 3 "angular_velocity"
```

---

## Summary

**Best ways to view calibration values:**

1. ✅ **Startup logs** - Easiest, shows values when node starts
2. ✅ **ROS2 param commands** - Good for checking while node is running
3. ✅ **params.yaml file** - Direct source of truth
4. ✅ **Published messages** - See the effect of calibration (values should be corrected)

The calibration values are loaded from `params.yaml` when the node starts and are printed to the console automatically (lines 29-33 in `mpu6500_node.cpp`).
