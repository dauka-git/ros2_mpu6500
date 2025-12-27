#include "ros2_mpu6500/mpu6500_hal.h"
#include <iostream>

Mpu6500Hal::Mpu6500Hal(const std::string &device, const std::uint8_t i2c_address)
    : m_device(device), m_i2c_address(i2c_address)
{
    m_i2c_file = open(m_device.c_str(), O_RDWR);
    if (m_i2c_file < 0) {
        std::cerr << "Failed to open I2C device: " << m_device << std::endl;
        throw std::runtime_error("I2C device open failed");
    }

    if (ioctl(m_i2c_file, I2C_SLAVE, m_i2c_address) < 0) {
        std::cerr << "Failed to set I2C slave address: " << static_cast<int>(m_i2c_address) << std::endl;
        close(m_i2c_file);
        throw std::runtime_error("I2C slave address set failed");
    }
}

Mpu6500Hal::~Mpu6500Hal()
{
    if (m_i2c_file >= 0) {
        close(m_i2c_file);
    }
}

Mpu6500Hal::Mpu6500_Error_t Mpu6500Hal::mpu6500_i2c_hal_read(const std::uint8_t reg, std::uint8_t aRxBuffer[], const std::uint16_t count)
{
    if (count == 1) {
        int32_t res = i2c_smbus_read_byte_data(m_i2c_file, reg);
        if (res < 0) {
            return MPU6500_ERR;
        }
        aRxBuffer[0] = static_cast<std::uint8_t>(res);
    } else {
        if (i2c_smbus_read_i2c_block_data(m_i2c_file, reg, count, aRxBuffer) != count) {
            return MPU6500_ERR;
        }
    }
    return MPU6500_OK;
}

Mpu6500Hal::Mpu6500_Error_t Mpu6500Hal::mpu6500_i2c_hal_write(const std::uint8_t reg, const std::uint8_t aTxBuffer[], const std::uint16_t count)
{
    if (count == 1) {
        if (i2c_smbus_write_byte_data(m_i2c_file, reg, aTxBuffer[0]) < 0) {
            return MPU6500_ERR;
        }
    } else {
        if (i2c_smbus_write_i2c_block_data(m_i2c_file, reg, count, aTxBuffer) < 0) {
            return MPU6500_ERR;
        }
    }
    return MPU6500_OK;
}