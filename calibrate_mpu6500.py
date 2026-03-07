#!/usr/bin/env python3
"""
MPU6500 Calibration Helper Script

This script helps calculate calibration offsets for the MPU6500 IMU sensor.
It reads recorded IMU data and calculates the average values to use as offsets.

Usage:
    1. Record IMU data: ros2 topic echo /imu/data > calibration_data.txt
    2. Run this script: python3 calibrate_mpu6500.py calibration_data.txt
    3. Copy the calculated offsets to config/params.yaml
"""

import sys
import re
from collections import defaultdict

def parse_imu_data(filename):
    """Parse ROS2 topic echo output and extract IMU values."""
    gyro_x_values = []
    gyro_y_values = []
    gyro_z_values = []
    accel_x_values = []
    accel_y_values = []
    accel_z_values = []
    
    current_axis = None
    current_type = None
    
    try:
        with open(filename, 'r') as f:
            for line in f:
                line = line.strip()
                
                # Detect angular_velocity section
                if 'angular_velocity:' in line:
                    current_type = 'gyro'
                    continue
                
                # Detect linear_acceleration section
                if 'linear_acceleration:' in line:
                    current_type = 'accel'
                    continue
                
                # Parse x, y, z values
                if current_type == 'gyro':
                    if 'x:' in line:
                        match = re.search(r'x:\s*([-+]?\d*\.?\d+)', line)
                        if match:
                            gyro_x_values.append(float(match.group(1)))
                    elif 'y:' in line:
                        match = re.search(r'y:\s*([-+]?\d*\.?\d+)', line)
                        if match:
                            gyro_y_values.append(float(match.group(1)))
                    elif 'z:' in line:
                        match = re.search(r'z:\s*([-+]?\d*\.?\d+)', line)
                        if match:
                            gyro_z_values.append(float(match.group(1)))
                
                elif current_type == 'accel':
                    if 'x:' in line:
                        match = re.search(r'x:\s*([-+]?\d*\.?\d+)', line)
                        if match:
                            accel_x_values.append(float(match.group(1)))
                    elif 'y:' in line:
                        match = re.search(r'y:\s*([-+]?\d*\.?\d+)', line)
                        if match:
                            accel_y_values.append(float(match.group(1)))
                    elif 'z:' in line:
                        match = re.search(r'z:\s*([-+]?\d*\.?\d+)', line)
                        if match:
                            accel_z_values.append(float(match.group(1)))
    
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found!")
        sys.exit(1)
    except Exception as e:
        print(f"Error parsing file: {e}")
        sys.exit(1)
    
    return {
        'gyro': {
            'x': gyro_x_values,
            'y': gyro_y_values,
            'z': gyro_z_values
        },
        'accel': {
            'x': accel_x_values,
            'y': accel_y_values,
            'z': accel_z_values
        }
    }

def calculate_averages(values):
    """Calculate average of a list of values."""
    if not values:
        return 0.0
    return sum(values) / len(values)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 calibrate_mpu6500.py <calibration_data.txt>")
        print("\nExample:")
        print("  1. Record data: ros2 topic echo /imu/data > calibration_data.txt")
        print("  2. Run script: python3 calibrate_mpu6500.py calibration_data.txt")
        sys.exit(1)
    
    filename = sys.argv[1]
    print(f"Parsing calibration data from: {filename}\n")
    
    # Parse the data
    data = parse_imu_data(filename)
    
    # Calculate averages
    gyro_avg_x = calculate_averages(data['gyro']['x'])
    gyro_avg_y = calculate_averages(data['gyro']['y'])
    gyro_avg_z = calculate_averages(data['gyro']['z'])
    
    accel_avg_x = calculate_averages(data['accel']['x'])
    accel_avg_y = calculate_averages(data['accel']['y'])
    accel_avg_z = calculate_averages(data['accel']['z'])
    
    # Print statistics
    print("=" * 60)
    print("CALIBRATION RESULTS")
    print("=" * 60)
    
    print(f"\nGyroscope Data (rad/s):")
    print(f"  Samples: X={len(data['gyro']['x'])}, Y={len(data['gyro']['y'])}, Z={len(data['gyro']['z'])}")
    print(f"  Average: X={gyro_avg_x:.6f}, Y={gyro_avg_y:.6f}, Z={gyro_avg_z:.6f}")
    print(f"  Min:     X={min(data['gyro']['x']) if data['gyro']['x'] else 0:.6f}, "
          f"Y={min(data['gyro']['y']) if data['gyro']['y'] else 0:.6f}, "
          f"Z={min(data['gyro']['z']) if data['gyro']['z'] else 0:.6f}")
    print(f"  Max:     X={max(data['gyro']['x']) if data['gyro']['x'] else 0:.6f}, "
          f"Y={max(data['gyro']['y']) if data['gyro']['y'] else 0:.6f}, "
          f"Z={max(data['gyro']['z']) if data['gyro']['z'] else 0:.6f}")
    
    print(f"\nAccelerometer Data (m/s²):")
    print(f"  Samples: X={len(data['accel']['x'])}, Y={len(data['accel']['y'])}, Z={len(data['accel']['z'])}")
    print(f"  Average: X={accel_avg_x:.6f}, Y={accel_avg_y:.6f}, Z={accel_avg_z:.6f}")
    print(f"  Min:     X={min(data['accel']['x']) if data['accel']['x'] else 0:.6f}, "
          f"Y={min(data['accel']['y']) if data['accel']['y'] else 0:.6f}, "
          f"Z={min(data['accel']['z']) if data['accel']['z'] else 0:.6f}")
    print(f"  Max:     X={max(data['accel']['x']) if data['accel']['x'] else 0:.6f}, "
          f"Y={max(data['accel']['y']) if data['accel']['y'] else 0:.6f}, "
          f"Z={max(data['accel']['z']) if data['accel']['z'] else 0:.6f}")
    
    # Calculate offsets (negative of average for gyro, average for accel)
    print("\n" + "=" * 60)
    print("RECOMMENDED OFFSETS FOR config/params.yaml")
    print("=" * 60)
    print("\n# Gyroscope offsets (negative of average)")
    print(f"gyro_x_offset: {-gyro_avg_x:.6f}")
    print(f"gyro_y_offset: {-gyro_avg_y:.6f}")
    print(f"gyro_z_offset: {-gyro_avg_z:.6f}")
    
    print("\n# Accelerometer offsets")
    print("# Option 1: Zero-g calibration (X/Y/Z all become 0 when flat)")
    print(f"accel_x_offset: {accel_avg_x:.6f}")
    print(f"accel_y_offset: {accel_avg_y:.6f}")
    print(f"accel_z_offset: {accel_avg_z:.6f}")
    
    print("\n# Option 2: Gravity calibration (Z shows 9.81 m/s² when flat)")
    print(f"accel_x_offset: {accel_avg_x:.6f}")
    print(f"accel_y_offset: {accel_avg_y:.6f}")
    print(f"accel_z_offset: 0.0  # Keep at 0 to preserve gravity reading")
    
    print("\n" + "=" * 60)
    print("INSTRUCTIONS:")
    print("=" * 60)
    print("1. Copy the gyroscope offsets above to config/params.yaml")
    print("2. Choose Option 1 or Option 2 for accelerometer offsets")
    print("3. Restart the node: ros2 launch ros2_mpu6500 ros2_mpu6500.launch.py")
    print("4. Verify: ros2 topic echo /imu/data")
    print("   - Gyro should be ~0 rad/s when stationary")
    print("   - Accel should match your calibration goal when flat")
    print("=" * 60)

if __name__ == '__main__':
    main()
