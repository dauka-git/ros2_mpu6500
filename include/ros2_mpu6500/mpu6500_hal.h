#ifndef MPU6500_HAL_H
#define MPU6500_HAL_H

#include <cstdint>
#include <string>

/* Hardware Specific Components */
extern "C" {
#include <errno.h>
#include <fcntl.h>
#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
}

class Mpu6500Hal {
public:
    Mpu6500Hal(const std::string &device, const std::uint8_t i2c_address);

    ~Mpu6500Hal();

    typedef int16_t Mpu6500_Error_t;

    static constexpr Mpu6500_Error_t MPU6500_ERR    = -1;
    static constexpr Mpu6500_Error_t MPU6500_OK     = 0;

    /**
     * @brief        Execute I2C read.
     * @details      Execute I2C read sequence.
     *
     * @param[in]    reg         Register address.
     * @param[in]    count       Number of bytes to read.
     * @param[out]   aRxBuffer   Array to which data will be stored.
     *
     * @return       Mpu6500_Error_t     Return code.
     *
     */
    Mpu6500_Error_t mpu6500_i2c_hal_read(const std::uint8_t reg, std::uint8_t aRxBuffer[], const std::uint16_t count);

    /**
     * @brief        Execute I2C write.
     * @details      Execute I2C write sequence.
     *
     * @param[in]    reg         Register address.
     * @param[in]    count       Number of bytes to write.
     * @param[in]    aTxBuffer   Array from which data will be sent.
     *
     * @return       Mpu6500_Error_t     Return code.
     *
     */
    Mpu6500_Error_t mpu6500_i2c_hal_write(const std::uint8_t reg, const std::uint8_t aTxBuffer[], const std::uint16_t count);

private:
    int m_i2c_file;
    std::string m_device;
    std::uint8_t m_i2c_address;
};

#endif  // MPU6500_HAL_H