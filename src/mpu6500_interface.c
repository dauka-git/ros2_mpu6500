#include "driver_mpu6500_interface.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <cstring>
#include <iostream>

static int i2c_fd = -1;
static const char* i2c_device = "/dev/i2c-1";
static uint8_t i2c_addr = 0x68; // Default MPU6500 address

uint8_t mpu6500_interface_iic_init(void)
{
    i2c_fd = open(i2c_device, O_RDWR);
    if (i2c_fd < 0) {
        std::cerr << "Failed to open I2C device" << std::endl;
        return 1;
    }
    if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0) {
        std::cerr << "Failed to set I2C slave address" << std::endl;
        close(i2c_fd);
        return 1;
    }
    return 0;
}

uint8_t mpu6500_interface_iic_deinit(void)
{
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
    return 0;
}

uint8_t mpu6500_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (i2c_fd < 0) return 1;

    if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) return 1;

    if (len == 1) {
        int32_t res = i2c_smbus_read_byte_data(i2c_fd, reg);
        if (res < 0) return 1;
        buf[0] = (uint8_t)res;
    } else {
        if (i2c_smbus_read_i2c_block_data(i2c_fd, reg, len, buf) != len) return 1;
    }
    return 0;
}

uint8_t mpu6500_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (i2c_fd < 0) return 1;

    if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) return 1;

    if (len == 1) {
        if (i2c_smbus_write_byte_data(i2c_fd, reg, buf[0]) < 0) return 1;
    } else {
        if (i2c_smbus_write_i2c_block_data(i2c_fd, reg, len, buf) < 0) return 1;
    }
    return 0;
}

uint8_t mpu6500_interface_spi_init(void)
{
    return 1; // SPI not implemented
}

uint8_t mpu6500_interface_spi_deinit(void)
{
    return 1; // SPI not implemented
}

uint8_t mpu6500_interface_spi_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
    return 1; // SPI not implemented
}

uint8_t mpu6500_interface_spi_write(uint8_t reg, uint8_t *buf, uint16_t len)
{
    return 1; // SPI not implemented
}

void mpu6500_interface_delay_ms(uint32_t ms)
{
    usleep(ms * 1000);
}

void mpu6500_interface_debug_print(const char *const fmt, ...)
{
    // Debug print - can be enabled if needed
}

void mpu6500_interface_receive_callback(uint8_t type)
{
    // Callback - not used
}

void mpu6500_interface_dmp_tap_callback(uint8_t count, uint8_t direction)
{
    // DMP callback - not used
}

void mpu6500_interface_dmp_orient_callback(uint8_t orientation)
{
    // DMP callback - not used
}