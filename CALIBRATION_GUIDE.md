# MPU6500 Calibration Guide

## Step 1: Build the Package (Fix "No Such Executable" Error)

The executable doesn't exist until you build the package. Follow these steps:

### 1. Navigate to your ROS2 workspace:
```bash
cd ~/ros2_ws  # or wherever your ROS2 workspace is
```

### 2. Build the package:
```bash
colcon build --packages-select ros2_mpu6500
```

### 3. Source the workspace:
```bash
source install/setup.bash
```

**Important:** You need to source this every time you open a new terminal, or add it to your `~/.bashrc`:
```bash
echo "source ~/ros2_ws/install/setup.bash" >> ~/.bashrc
```

### 4. Verify the executable exists:
```bash
ros2 pkg executables ros2_mpu6500
# Should show: ros2_mpu6500 ros2_mpu6500
```

### 5. Now you can run it:
```bash
ros2 run ros2_mpu6500 ros2_mpu6500
# OR with debug logging:
ros2 run ros2_mpu6500 ros2_mpu6500 --ros-args --log-level debug
```

---

## Step 2: Calibration Process

There are **two ways** to calibrate:

### Method 1: Manual Calibration (Using Helper Script)

See the `calibrate_mpu6500.py` script below - it automates the calculation.

### Method 2: Manual Calculation

#### A. Gyroscope Calibration

**Goal:** When sensor is stationary, gyro values should be 0 rad/s.

1. **Set all offsets to 0 in `config/params.yaml`:**
   ```yaml
   gyro_x_offset: 0.0
   gyro_y_offset: 0.0
   gyro_z_offset: 0.0
   ```

2. **Place IMU on stable, stationary surface** (don't move it!)

3. **Start the node:**
   ```bash
   ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py
   ```

4. **Record data for 30-60 seconds:**
   ```bash
   # In another terminal
   ros2 topic echo /imu/data > gyro_calibration_data.txt
   # Wait 30-60 seconds, then press Ctrl+C
   ```

5. **Calculate averages:**
   - Open `gyro_calibration_data.txt`
   - Extract all `angular_velocity.x`, `angular_velocity.y`, `angular_velocity.z` values
   - Calculate average of each axis
   - **The offset is the negative of the average**

6. **Update `config/params.yaml`:**
   ```yaml
   gyro_x_offset: -0.0123  # Negative of average X
   gyro_y_offset: -0.0045  # Negative of average Y
   gyro_z_offset: -0.0067  # Negative of average Z
   ```

#### B. Accelerometer Calibration

**Goal:** When sensor is flat and level:
- X and Y should be 0 m/s²
- Z should be 0 m/s² (if you want zero-g) OR 9.81 m/s² (if you want gravity)

1. **Set accel offsets to 0 in `config/params.yaml`:**
   ```yaml
   accel_x_offset: 0.0
   accel_y_offset: 0.0
   accel_z_offset: 0.0
   ```

2. **Place IMU on flat, level surface** (perfectly horizontal)

3. **Record data for 30-60 seconds:**
   ```bash
   ros2 topic echo /imu/data > accel_calibration_data.txt
   # Wait 30-60 seconds, then press Ctrl+C
   ```

4. **Calculate averages:**
   - Extract all `linear_acceleration.x`, `linear_acceleration.y`, `linear_acceleration.z` values
   - Calculate average of each axis

5. **Update `config/params.yaml`:**
   ```yaml
   # If you want X and Y to be 0 when flat:
   accel_x_offset: 0.1234  # Average X value
   accel_y_offset: -0.0567  # Average Y value
   
   # If you want Z to be 0 (zero-g calibration):
   accel_z_offset: 9.8100  # Average Z value (should be ~9.81)
   
   # OR if you want Z to show gravity (9.81 m/s²):
   accel_z_offset: 0.0  # Keep at 0
   ```

---

## Step 3: Verify Calibration

1. **Restart the node with new offsets:**
   ```bash
   ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py
   ```

2. **Check the startup logs** - you should see your offsets printed:
   ```
   [INFO] [mpu6500_sensor]: IMU Calibration Offsets:
   [INFO] [mpu6500_sensor]:   Gyro: [-0.0123, -0.0045, -0.0067] rad/s
   [INFO] [mpu6500_sensor]:   Accel: [0.1234, -0.0567, 9.8100] m/s²
   ```

3. **Verify values are correct:**
   ```bash
   ros2 topic echo /imu/data
   ```
   
   - When stationary: gyro values should be **near 0 rad/s**
   - When flat: accel X/Y should be **near 0 m/s²** (if calibrated for zero)
   - When flat: accel Z should be **near 0 or 9.81 m/s²** (depending on your calibration goal)

---

## Quick Calibration Checklist

- [ ] Package built (`colcon build --packages-select ros2_mpu6500`)
- [ ] Workspace sourced (`source install/setup.bash`)
- [ ] Node runs successfully
- [ ] IMU data is being published (`ros2 topic echo /imu/data`)
- [ ] Recorded calibration data (30-60 seconds, sensor stationary/flat)
- [ ] Calculated offsets (averages)
- [ ] Updated `config/params.yaml` with offsets
- [ ] Restarted node and verified corrected values

---

## Troubleshooting

### "No such executable" Error
- **Solution:** Build the package first (see Step 1 above)

### "Failed to open I2C device"
- Check I2C permissions: `ls -la /dev/i2c-7`
- Fix: `sudo chmod 666 /dev/i2c-7` or add user to i2c group

### No data published
- Check node is running: `ros2 node list`
- Check topic exists: `ros2 topic list`
- Check for errors in node output

### Calibration values don't work
- Make sure you're using **negative** of average for gyro offsets
- Verify units: gyro in rad/s, accel in m/s²
- Check that sensor was truly stationary during recording
