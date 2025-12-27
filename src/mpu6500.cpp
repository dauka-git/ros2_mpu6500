#include "ros2_mpu6500/mpu6500.h"
#include <iostream>

Mpu6500::Mpu6500(const std::string &device, int i2c_address)
    : initialized_(false)
{
    // Initialize the MPU6500 handle
    if (mpu6500_init(&handle_) != 0) {
        std::cerr << "Failed to initialize MPU6500 handle" << std::endl;
        return;
    }

    // Set interface to I2C
    if (mpu6500_set_interface(&handle_, MPU6500_INTERFACE_IIC) != 0) {
        std::cerr << "Failed to set I2C interface" << std::endl;
        return;
    }

    // Set address
    mpu6500_address_t addr = (i2c_address == MPU6500_ADDRESS_AD0_LOW) ? MPU6500_ADDRESS_AD0_LOW : MPU6500_ADDRESS_AD0_HIGH;
    if (mpu6500_set_addr_pin(&handle_, addr) != 0) {
        std::cerr << "Failed to set address pin" << std::endl;
        return;
    }

    // Initialize the device
    if (mpu6500_init(&handle_) != 0) {
        std::cerr << "Failed to initialize MPU6500 device" << std::endl;
        return;
    }

    initialized_ = true;
}

Mpu6500::~Mpu6500()
{
    if (initialized_) {
        mpu6500_deinit(&handle_);
    }
}

Mpu6500Hal::Mpu6500_Error_t Mpu6500::Mpu6500_GetAccelData(Mpu6500_AccelData_t &AccelData)
{
    if (!initialized_) return Mpu6500Hal::MPU6500_ERR;

    int16_t accel_raw[3];
    float accel_g[3];
    int16_t gyro_raw[3];
    float gyro_dps[3];
    uint16_t len;

    uint8_t res = mpu6500_read(&handle_, accel_raw, accel_g, gyro_raw, gyro_dps, &len);
    if (res != 0) {
        return Mpu6500Hal::MPU6500_ERR;
    }

    AccelData.Accel_X = accel_g[0] * 9.81; // Convert g to m/s²
    AccelData.Accel_Y = accel_g[1] * 9.81;
    AccelData.Accel_Z = accel_g[2] * 9.81;

    return Mpu6500Hal::MPU6500_OK;
}

Mpu6500Hal::Mpu6500_Error_t Mpu6500::Mpu6500_GetGyroData(Mpu6500_GyroData_t &GyroData)
{
    if (!initialized_) return Mpu6500Hal::MPU6500_ERR;

    int16_t accel_raw[3];
    float accel_g[3];
    int16_t gyro_raw[3];
    float gyro_dps[3];
    uint16_t len;

    uint8_t res = mpu6500_read(&handle_, accel_raw, accel_g, gyro_raw, gyro_dps, &len);
    if (res != 0) {
        return Mpu6500Hal::MPU6500_ERR;
    }

    // Convert degrees per second to radians per second
    GyroData.Gyro_X = gyro_dps[0] * M_PI / 180.0;
    GyroData.Gyro_Y = gyro_dps[1] * M_PI / 180.0;
    GyroData.Gyro_Z = gyro_dps[2] * M_PI / 180.0;

    return Mpu6500Hal::MPU6500_OK;
}