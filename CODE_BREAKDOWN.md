# MPU6500 ROS2 Driver - Complete Code Breakdown & Debugging Guide

## 📋 Table of Contents
1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Component Breakdown](#component-breakdown)
4. [Data Flow](#data-flow)
5. [Debugging Strategies](#debugging-strategies)

---

## Overview

This is a **ROS2 driver package** for the **MPU6500 6-axis IMU sensor** (3-axis accelerometer + 3-axis gyroscope). It reads sensor data via I2C and publishes it as ROS2 `sensor_msgs/Imu` messages.

**Key Features:**
- I2C communication with MPU6500 sensor
- Configurable calibration offsets
- 100Hz publish rate (configurable)
- Standard ROS2 sensor_msgs/Imu message format

---

## Architecture

The code follows a **layered architecture**:

```
┌─────────────────────────────────────┐
│   ROS2 Node Layer (mpu6500_node)    │  ← Publishes to /imu/data topic
├─────────────────────────────────────┤
│   Sensor Wrapper (mpu6500.cpp)      │  ← C++ interface to driver
├─────────────────────────────────────┤
│   Hardware Abstraction (mpu6500_hal)│  ← I2C read/write operations
├─────────────────────────────────────┤
│   Interface Layer (mpu6500_interface)│ ← C interface to libdriver
├─────────────────────────────────────┤
│   External Driver (driver_mpu6500)  │  ← Third-party MPU6500 driver
└─────────────────────────────────────┘
```

---

## Component Breakdown

### 1. **Mpu6500Node** (`mpu6500_node.h/cpp`)
**Purpose:** ROS2 node that orchestrates sensor reading and message publishing.

**Key Components:**
- **Publisher:** Publishes `sensor_msgs/Imu` messages to `/imu/data` topic
- **Timer:** Triggers data reading at 100Hz (10ms interval)
- **Calibration Offsets:** Stores 6 offset values (3 gyro, 3 accel)
- **Mpu6500 Device:** Wrapper object that interfaces with the sensor

**What it does:**
1. Declares ROS2 parameters for calibration offsets
2. Creates publisher and timer
3. Every 10ms: Reads accel/gyro data → Applies offsets → Publishes message

**Key Code Sections:**
```cpp
// Constructor: Sets up node, reads parameters, creates publisher/timer
Mpu6500Node::Mpu6500Node(const std::string& name)

// Callback: Called every 10ms to read and publish data
void Mpu6500Node::ImuPubCallback()
```

---

### 2. **Mpu6500** (`mpu6500.h/cpp`)
**Purpose:** C++ wrapper class that provides high-level sensor interface.

**Key Components:**
- **Handle:** Connection to the underlying driver (`mpu6500_handle_t`)
- **Initialization State:** Tracks if sensor is ready
- **Data Structures:** `Mpu6500_AccelData_t`, `Mpu6500_GyroData_t`

**What it does:**
1. **Initialization:**
   - Links interface functions to driver
   - Sets I2C interface mode
   - Configures sensor ranges (gyro: ±250°/s, accel: ±2g)
   - Sets sample rate (100Hz), filter (92Hz), clock source
   
2. **Data Reading:**
   - `Mpu6500_GetAccelData()`: Reads accelerometer, converts g → m/s²
   - `Mpu6500_GetGyroData()`: Reads gyroscope, converts °/s → rad/s

**Key Code Sections:**
```cpp
// Constructor: Initializes sensor with all settings
Mpu6500::Mpu6500(const std::string &device, int i2c_address)

// Read accelerometer data (returns m/s²)
Mpu6500Hal::Mpu6500_Error_t Mpu6500_GetAccelData(Mpu6500_AccelData_t &AccelData)

// Read gyroscope data (returns rad/s)
Mpu6500Hal::Mpu6500_Error_t Mpu6500_GetGyroData(Mpu6500_GyroData_t &GyroData)
```

---

### 3. **Mpu6500Hal** (`mpu6500_hal.h/cpp`)
**Purpose:** Hardware Abstraction Layer - handles low-level I2C communication.

**Key Components:**
- **I2C File Descriptor:** Linux file handle to I2C device (`/dev/i2c-7`)
- **I2C Address:** 7-bit device address (0x68 or 0x69)
- **Read/Write Functions:** Direct I2C register access

**What it does:**
1. Opens I2C device file (`/dev/i2c-7`)
2. Sets I2C slave address via `ioctl()`
3. Provides read/write functions using Linux `i2c_smbus` API

**Key Code Sections:**
```cpp
// Constructor: Opens I2C device and sets address
Mpu6500Hal::Mpu6500Hal(const std::string &device, const std::uint8_t i2c_address)

// Read I2C register(s)
Mpu6500_Error_t mpu6500_i2c_hal_read(uint8_t reg, uint8_t aRxBuffer[], uint16_t count)

// Write I2C register(s)
Mpu6500_Error_t mpu6500_i2c_hal_write(uint8_t reg, uint8_t aTxBuffer[], uint16_t count)
```

**Note:** This class is defined but **NOT USED** in the current code! The actual I2C operations go through `mpu6500_interface.c`.

---

### 4. **mpu6500_interface.c**
**Purpose:** C interface layer that bridges the external `driver_mpu6500` library with the system.

**Key Components:**
- **Static I2C File Descriptor:** `i2c_fd` (global state)
- **I2C Device Path:** Hardcoded to `/dev/i2c-7`
- **I2C Address:** Default 0x68 (can change dynamically)

**What it does:**
1. **I2C Initialization:** Opens `/dev/i2c-7`, sets slave address
2. **I2C Read/Write:** Implements functions called by `driver_mpu6500`
3. **Address Handling:** Converts 8-bit addresses (from driver) to 7-bit (for Linux)
4. **SPI Functions:** Stubbed out (returns error - not implemented)
5. **Debug/Utility:** Delay, debug print, interrupt callbacks

**Key Functions:**
```c
// Initialize I2C bus
uint8_t mpu6500_interface_iic_init(void)

// Read from I2C register
uint8_t mpu6500_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)

// Write to I2C register
uint8_t mpu6500_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
```

**Critical Detail:** The I2C device path is **hardcoded** to `/dev/i2c-7` on line 16, but the config file might specify a different bus!

---

### 5. **Configuration Files**

#### `config/params.yaml`
**Purpose:** ROS2 parameter file for sensor configuration.

**Parameters:**
- `i2c_device`: I2C bus path (default: `/dev/i2c-7`)
- `i2c_address`: Device address (0x68 or 0x69)
- `publish_rate_hz`: Publishing frequency (currently not used - hardcoded to 100Hz)
- `frame_id`: TF frame name (currently hardcoded to "imu_link")
- `gyro_x/y/z_offset`: Gyroscope calibration offsets
- `accel_x/y/z_offset`: Accelerometer calibration offsets

**Issue:** Some parameters are declared but not actually used in the code!

#### `launch/ros2_mpu6500.launch.py`
**Purpose:** ROS2 launch file to start the node.

**What it does:**
- Loads parameter file
- Creates and starts the `mpu6500_sensor` node
- Outputs logs to screen

---

## Data Flow

### Initialization Flow:
```
main() 
  → rclcpp::init()
  → Mpu6500Node("mpu6500_sensor")
    → Mpu6500() constructor
      → Links interface functions
      → mpu6500_set_interface(I2C)
      → mpu6500_set_addr_pin(0x68)
      → mpu6500_init()
        → Calls mpu6500_interface_iic_init()
          → Opens /dev/i2c-7
          → Sets I2C slave address
      → Configure ranges, filters, sample rate
  → Create publisher ("imu/data")
  → Create timer (10ms → ImuPubCallback)
  → rclcpp::spin()
```

### Runtime Data Flow (Every 10ms):
```
Timer triggers ImuPubCallback()
  → mpu6500_dev_->Mpu6500_GetAccelData()
    → mpu6500_read() [from driver]
      → mpu6500_interface_iic_read() [reads registers]
      → Driver converts raw → g
    → Convert g → m/s² (×9.80665)
  → mpu6500_dev_->Mpu6500_GetGyroData()
    → mpu6500_read() [from driver]
      → mpu6500_interface_iic_read() [reads registers]
      → Driver converts raw → °/s
    → Convert °/s → rad/s (×π/180)
  → Apply calibration offsets
  → Fill sensor_msgs/Imu message
  → publisher_->publish(message)
```

---

## Debugging Strategies

### 🔍 **1. Debug I2C Hardware Connection**

**Problem:** Sensor not detected or communication fails.

**Debug Steps:**

1. **Check I2C device exists:**
   ```bash
   ls -la /dev/i2c-*
   # Should see /dev/i2c-7 (or your bus number)
   ```

2. **Check I2C permissions:**
   ```bash
   ls -la /dev/i2c-7
   # Should be readable/writable by your user
   # If not: sudo chmod 666 /dev/i2c-7
   ```

3. **Scan I2C bus for devices:**
   ```bash
   sudo i2cdetect -y -r 7
   # Should show 0x68 or 0x69 (depending on AD0 pin)
   ```

4. **Read WHO_AM_I register manually:**
   ```bash
   sudo i2cget -y 7 0x68 0x75
   # Should return 0x70 (MPU6500 device ID)
   ```

5. **Add debug prints in `mpu6500_interface.c`:**
   ```c
   // In mpu6500_interface_iic_init()
   printf("DEBUG: Opening I2C device: %s\n", i2c_device);
   printf("DEBUG: I2C file descriptor: %d\n", i2c_fd);
   printf("DEBUG: Setting address: 0x%02X\n", current_i2c_addr);
   ```

**Common Issues:**
- Wrong I2C bus number (check with `i2cdetect`)
- Wrong I2C address (check AD0 pin connection)
- Permission denied (add user to `i2c` group)
- Device not powered (check 3.3V supply)

---

### 🔍 **2. Debug Sensor Initialization**

**Problem:** Node crashes or fails to start.

**Debug Steps:**

1. **Add error checking in `mpu6500.cpp` constructor:**
   ```cpp
   // After each mpu6500_* call:
   if (result != 0) {
       std::cerr << "ERROR: mpu6500_set_gyroscope_range failed with code: " 
                 << result << std::endl;
       // Print which step failed
   }
   ```

2. **Check initialization sequence:**
   - Enable debug prints in `mpu6500_interface_debug_print()` (already prints)
   - Look for "i2c init success" message
   - Check for any error messages before crash

3. **Verify driver library is linked:**
   ```bash
   ldd build/ros2_mpu6500/ros2_mpu6500
   # Should show driver_mpu6500 library
   ```

4. **Test with minimal initialization:**
   - Comment out non-essential config steps
   - Test one configuration at a time

**Common Issues:**
- Driver library not found
- I2C interface not set correctly
- Sensor not responding (power/connection issue)
- Wrong I2C address

---

### 🔍 **3. Debug Data Reading**

**Problem:** No data published, or data is invalid/zero.

**Debug Steps:**

1. **Add debug prints in `Mpu6500_GetAccelData()` and `Mpu6500_GetGyroData()`:**
   ```cpp
   // In mpu6500.cpp, after mpu6500_read():
   std::cout << "DEBUG: Raw accel [g]: " 
             << accel_g[0] << ", " << accel_g[1] << ", " << accel_g[2] << std::endl;
   std::cout << "DEBUG: Raw gyro [dps]: " 
             << gyro_dps[0] << ", " << gyro_dps[1] << ", " << gyro_dps[2] << std::endl;
   std::cout << "DEBUG: Converted accel [m/s²]: " 
             << AccelData.Accel_X << ", " << AccelData.Accel_Y << ", " 
             << AccelData.Accel_Z << std::endl;
   ```

2. **Check if callback is being called:**
   ```cpp
   // In ImuPubCallback(), add at the start:
   static int call_count = 0;
   if (++call_count % 100 == 0) {
       RCLCPP_INFO(this->get_logger(), "Callback called %d times", call_count);
   }
   ```

3. **Verify data conversion:**
   - Check raw values from driver (should be in g and °/s)
   - Verify conversion factors (9.80665 for accel, π/180 for gyro)
   - Check for overflow/underflow

4. **Test I2C reads manually:**
   ```bash
   # Read accelerometer registers (0x3B-0x40)
   sudo i2cdump -y 7 0x68
   # Should show non-zero values when sensor moves
   ```

**Common Issues:**
- `mpu6500_read()` returns error (check return code)
- Data length is 0 (sensor not reading)
- Wrong conversion factors
- Sensor in sleep mode

---

### 🔍 **4. Debug ROS2 Publishing**

**Problem:** Node runs but no messages on topic.

**Debug Steps:**

1. **Check if node is running:**
   ```bash
   ros2 node list
   # Should show: /mpu6500_sensor
   ```

2. **Check if topic exists:**
   ```bash
   ros2 topic list
   # Should show: /imu/data
   ```

3. **Check topic info:**
   ```bash
   ros2 topic info /imu/data
   # Should show publisher: /mpu6500_sensor
   ```

4. **Check publishing rate:**
   ```bash
   ros2 topic hz /imu/data
   # Should show ~100 Hz
   ```

5. **Echo topic to see messages:**
   ```bash
   ros2 topic echo /imu/data
   # Should show IMU messages with data
   ```

6. **Add debug in `ImuPubCallback()`:**
   ```cpp
   // After reading data:
   RCLCPP_DEBUG(this->get_logger(), 
                "Publishing: accel=[%.3f, %.3f, %.3f], gyro=[%.3f, %.3f, %.3f]",
                message.linear_acceleration.x, message.linear_acceleration.y, 
                message.linear_acceleration.z,
                message.angular_velocity.x, message.angular_velocity.y, 
                message.angular_velocity.z);
   ```

**Common Issues:**
- Publisher not created
- Timer not triggering
- Errors in callback preventing publish
- Wrong topic name

---

### 🔍 **5. Debug Calibration Offsets**

**Problem:** Sensor data has constant bias.

**Debug Steps:**

1. **Check if offsets are loaded:**
   ```cpp
   // In constructor, after reading parameters:
   RCLCPP_INFO(this->get_logger(), "Loaded offsets: gyro=[%.4f, %.4f, %.4f]",
               gyro_x_offset_, gyro_y_offset_, gyro_z_offset_);
   ```

2. **Record data without offsets:**
   - Set all offsets to 0.0
   - Record stationary data for 30 seconds
   - Calculate average (this is your bias)

3. **Apply offsets:**
   - Set offsets to negative of average
   - Verify data is now near zero when stationary

4. **Check offset application:**
   ```cpp
   // In ImuPubCallback(), before applying offset:
   RCLCPP_DEBUG(this->get_logger(), "Before offset: accel_z=%.3f", AccelData.Accel_Z);
   // After applying offset:
   RCLCPP_DEBUG(this->get_logger(), "After offset: accel_z=%.3f", 
                message.linear_acceleration.z);
   ```

**Common Issues:**
- Offsets not loaded from parameters
- Wrong sign (should subtract offset)
- Offsets too large (check units)

---

### 🔍 **6. Debug Configuration Issues**

**Problem:** Parameters not being used or wrong values.

**Debug Steps:**

1. **Check parameter loading:**
   ```bash
   ros2 param list /mpu6500_sensor
   ros2 param get /mpu6500_sensor gyro_x_offset
   ```

2. **Verify I2C device path:**
   - Config says `/dev/i2c-7`
   - But `mpu6500_interface.c` hardcodes `/dev/i2c-7` (line 16)
   - **Issue:** Config parameter is ignored!

3. **Check publish rate:**
   - Config has `publish_rate_hz: 100`
   - But timer is hardcoded to `10ms` (100Hz)
   - **Issue:** Config parameter is ignored!

4. **Add parameter validation:**
   ```cpp
   // In Mpu6500Node constructor:
   auto publish_rate = this->declare_parameter<double>("publish_rate_hz", 100.0);
   auto period_ms = 1000.0 / publish_rate;
   timer_ = this->create_wall_timer(
       std::chrono::milliseconds(static_cast<int>(period_ms)),
       std::bind(&Mpu6500Node::ImuPubCallback, this));
   ```

---

### 🔍 **7. Debug Performance Issues**

**Problem:** Low publish rate, missed messages, or high CPU usage.

**Debug Steps:**

1. **Measure callback execution time:**
   ```cpp
   void Mpu6500Node::ImuPubCallback() {
       auto start = std::chrono::high_resolution_clock::now();
       
       // ... existing code ...
       
       auto end = std::chrono::high_resolution_clock::now();
       auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
       if (duration.count() > 1000) {  // > 1ms
           RCLCPP_WARN(this->get_logger(), "Callback took %ld μs", duration.count());
       }
   }
   ```

2. **Check I2C read time:**
   - I2C operations are blocking
   - Each read takes ~1-2ms
   - Two reads per callback = 2-4ms total

3. **Monitor system resources:**
   ```bash
   top -p $(pgrep ros2_mpu6500)
   # Check CPU usage
   ```

4. **Reduce publish rate if needed:**
   - Change timer from 10ms to 20ms (50Hz)
   - Or 100ms (10Hz) for testing

---

### 🔍 **8. Debug Build/Compilation Issues**

**Problem:** Code doesn't compile or link.

**Debug Steps:**

1. **Check CMakeLists.txt:**
   - Verify all source files are listed
   - Check include directories
   - Verify external driver path

2. **Check for missing dependencies:**
   ```bash
   # Install I2C development library
   sudo apt-get install libi2c-dev
   ```

3. **Clean and rebuild:**
   ```bash
   cd ~/ros2_ws
   rm -rf build/ros2_mpu6500 install/ros2_mpu6500
   colcon build --packages-select ros2_mpu6500
   ```

4. **Check compiler errors:**
   - Look for missing headers
   - Check C/C++ linkage (extern "C" blocks)
   - Verify driver library exists

---

## 🛠️ Quick Debug Checklist

When something doesn't work, check in this order:

1. ✅ **Hardware:**
   - [ ] I2C device exists (`ls /dev/i2c-*`)
   - [ ] Sensor detected (`i2cdetect -y -r 7`)
   - [ ] Correct I2C address (0x68 or 0x69)
   - [ ] Power supply (3.3V)

2. ✅ **Permissions:**
   - [ ] I2C device readable (`ls -la /dev/i2c-7`)
   - [ ] User in `i2c` group

3. ✅ **Software:**
   - [ ] Node running (`ros2 node list`)
   - [ ] Topic exists (`ros2 topic list`)
   - [ ] Messages publishing (`ros2 topic echo /imu/data`)
   - [ ] No errors in logs

4. ✅ **Data:**
   - [ ] Values are non-zero
   - [ ] Values change when sensor moves
   - [ ] Units are correct (m/s², rad/s)
   - [ ] Offsets applied correctly

---

## 📝 Known Issues & Improvements

1. **I2C device path hardcoded:** `mpu6500_interface.c` line 16 hardcodes `/dev/i2c-7`, ignoring config parameter.

2. **Publish rate hardcoded:** Timer is 10ms (100Hz), ignoring `publish_rate_hz` parameter.

3. **Frame ID hardcoded:** Always "imu_link", ignoring `frame_id` parameter.

4. **Mpu6500Hal unused:** The HAL class is defined but never instantiated - I2C goes through interface.c instead.

5. **No error recovery:** If I2C read fails, node continues but publishes no data (should retry or shutdown gracefully).

6. **Double I2C initialization:** Both `Mpu6500Hal` and `mpu6500_interface.c` can open I2C (though only interface.c is used).

---

## 🔧 Recommended Debugging Tools

- **i2c-tools:** `i2cdetect`, `i2cget`, `i2cset`, `i2cdump`
- **ROS2 tools:** `ros2 topic echo`, `ros2 topic hz`, `ros2 node info`
- **System tools:** `strace` (trace system calls), `dmesg` (kernel messages)
- **GDB:** For debugging crashes
- **Valgrind:** For memory leak detection

---

## 📚 Additional Resources

- MPU6500 Datasheet: Register map and specifications
- Linux I2C Documentation: `/dev/i2c-*` interface
- ROS2 sensor_msgs/Imu: Message format documentation
