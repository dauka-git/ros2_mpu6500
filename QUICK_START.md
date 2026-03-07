# Quick Start Guide - Build & Calibrate

## Fix "No Such Executable" Error

```bash
# 1. Navigate to ROS2 workspace
cd ~/ros2_ws  # or your workspace path

# 2. Build the package
colcon build --packages-select ros2_mpu6500

# 3. Source the workspace
source install/setup.bash

# 4. Now you can run it!
ros2 run ros2_mpu6500 ros2_mpu6500
```

---

## Quick Calibration Process

### Step 1: Record Calibration Data

```bash
# Terminal 1: Start the node
ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py

# Terminal 2: Record data (keep sensor STATIONARY and FLAT)
ros2 topic echo /imu/data > calibration_data.txt
# Wait 30-60 seconds, then press Ctrl+C
```

### Step 2: Calculate Offsets

```bash
# Use the helper script
python3 calibrate_mpu6500.py calibration_data.txt

# OR manually calculate averages from the file
```

### Step 3: Update Config

Edit `config/params.yaml` with the calculated offsets:

```yaml
gyro_x_offset: -0.0123  # Negative of average
gyro_y_offset: -0.0045
gyro_z_offset: -0.0067
accel_x_offset: 0.1234  # Average value
accel_y_offset: -0.0567
accel_z_offset: 9.8100  # Or 0.0 if you want gravity
```

### Step 4: Verify

```bash
# Restart node
ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py

# Check values
ros2 topic echo /imu/data
# Gyro should be ~0 when stationary
# Accel should match your calibration goal
```

---

## Common Commands

```bash
# Build
colcon build --packages-select ros2_mpu6500

# Source (do this every time you open a new terminal)
source install/setup.bash

# Run node
ros2 run ros2_mpu6500 ros2_mpu6500

# Run with launch file
ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py

# View data
ros2 topic echo /imu/data

# Check publish rate
ros2 topic hz /imu/data

# View parameters
ros2 param get /mpu6500_sensor gyro_x_offset
```
