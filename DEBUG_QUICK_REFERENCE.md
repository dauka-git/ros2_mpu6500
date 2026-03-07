# MPU6500 Debugging Quick Reference

## 🚨 Common Problems & Quick Fixes

### Problem: "Failed to open I2C device"
```bash
# Check device exists
ls /dev/i2c-*

# Fix permissions
sudo chmod 666 /dev/i2c-7
# OR add user to i2c group
sudo usermod -aG i2c $USER
# Then logout/login
```

### Problem: "Failed to set I2C slave address"
```bash
# Scan I2C bus to find sensor
sudo i2cdetect -y -r 7
# Should show 0x68 or 0x69

# If not found:
# - Check power (3.3V)
# - Check connections (SDA, SCL)
# - Try other I2C bus numbers
```

### Problem: Node starts but no data published
```bash
# Check node is running
ros2 node list

# Check topic exists
ros2 topic list

# Check publishing rate
ros2 topic hz /imu/data

# View messages
ros2 topic echo /imu/data
```

### Problem: Data is all zeros
```bash
# Check if sensor is reading
sudo i2cdump -y 7 0x68 | head -20
# Should show non-zero values

# Check node logs for errors
ros2 run ros2_mpu6500 ros2_mpu6500
```

### Problem: Data is noisy or has constant bias
```yaml
# Edit config/params.yaml
# Record stationary data, calculate average, set negative as offset:
gyro_x_offset: -0.0123  # Negative of average reading
accel_z_offset: -9.81   # If Z should be 0 when flat
```

---

## 🔍 Debug Commands Cheat Sheet

### Hardware Verification
```bash
# List I2C devices
ls /dev/i2c-*

# Scan I2C bus 7
sudo i2cdetect -y -r 7

# Read WHO_AM_I register (should return 0x70)
sudo i2cget -y 7 0x68 0x75

# Dump all registers
sudo i2cdump -y 7 0x68
```

### ROS2 Verification
```bash
# List nodes
ros2 node list

# List topics
ros2 topic list

# Topic info
ros2 topic info /imu/data

# View messages
ros2 topic echo /imu/data

# Check publish rate
ros2 topic hz /imu/data

# View node parameters
ros2 param list /mpu6500_sensor
ros2 param get /mpu6500_sensor gyro_x_offset
```

### Build & Run
```bash
# Clean build
cd ~/ros2_ws
rm -rf build/ros2_mpu6500 install/ros2_mpu6500
colcon build --packages-select ros2_mpu6500

# Source workspace
source install/setup.bash

# Run node
ros2 run ros2_mpu6500 ros2_mpu6500

# OR use launch file
ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py
```

---

## 🐛 Adding Debug Prints

### In `mpu6500_node.cpp` - Check callback execution:
```cpp
void Mpu6500Node::ImuPubCallback() {
    static int count = 0;
    RCLCPP_DEBUG(this->get_logger(), "Callback #%d", ++count);
    // ... rest of code
}
```

### In `mpu6500.cpp` - Check data reading:
```cpp
Mpu6500Hal::Mpu6500_Error_t Mpu6500::Mpu6500_GetAccelData(...) {
    // After mpu6500_read():
    std::cout << "DEBUG: accel_g = [" 
              << accel_g[0] << ", " << accel_g[1] << ", " << accel_g[2] 
              << "]" << std::endl;
    // ... rest of code
}
```

### In `mpu6500_interface.c` - Check I2C operations:
```c
uint8_t mpu6500_interface_iic_read(...) {
    printf("DEBUG: Reading reg 0x%02X, len=%d\n", reg, len);
    // ... rest of code
    printf("DEBUG: Read result: 0x%02X\n", buf[0]);
    return 0;
}
```

---

## 📊 Expected Values

### When Stationary (flat, not moving):
- **Accelerometer Z:** ~9.81 m/s² (1g upward)
- **Accelerometer X, Y:** ~0 m/s²
- **Gyroscope X, Y, Z:** ~0 rad/s

### When Moving:
- **Accelerometer:** Changes with acceleration
- **Gyroscope:** Changes with rotation

### Publish Rate:
- Should be ~100 Hz (10ms intervals)
- Check with: `ros2 topic hz /imu/data`

---

## 🔧 Code Locations for Common Fixes

### Change I2C Bus:
**File:** `src/mpu6500_interface.c`  
**Line:** 16  
**Change:** `static const char* i2c_device = "/dev/i2c-7";`

### Change Publish Rate:
**File:** `src/mpu6500_node.cpp`  
**Line:** 37  
**Change:** `timer_ = this->create_wall_timer(20ms, ...);` (for 50Hz)

### Change Sensor Ranges:
**File:** `src/mpu6500.cpp`  
**Lines:** 53, 60  
**Change:** `MPU6500_GYROSCOPE_RANGE_250DPS` → `MPU6500_GYROSCOPE_RANGE_2000DPS`

### Change Frame ID:
**File:** `src/mpu6500_node.cpp`  
**Line:** 44  
**Change:** `message.header.frame_id = "imu_link";`

---

## ⚠️ Known Issues

1. **I2C device path hardcoded** in `mpu6500_interface.c` (ignores config)
2. **Publish rate hardcoded** to 100Hz (ignores config parameter)
3. **Frame ID hardcoded** to "imu_link" (ignores config parameter)
4. **Mpu6500Hal class unused** (I2C goes through interface.c instead)

---

## 📞 Getting Help

1. Check node logs: `ros2 run ros2_mpu6500 ros2_mpu6500`
2. Check system logs: `dmesg | tail -20`
3. Verify hardware: `sudo i2cdetect -y -r 7`
4. Check ROS2: `ros2 topic echo /imu/data`
