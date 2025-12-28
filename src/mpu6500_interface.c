/**
 * MPU6500 Interface Implementation for Jetson I2C Bus 7
 */

#include "driver_mpu6500_interface.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static int i2c_fd = -1;
static const char* i2c_device = "/dev/i2c-7";  // I2C bus 7 for Jetson
static uint8_t current_i2c_addr = 0x68;

/**
 * @brief  interface iic bus init
 * @return status code
 *         - 0 success
 *         - 1 iic init failed
 * @note   none
 */
uint8_t mpu6500_interface_iic_init(void)
{
    i2c_fd = open(i2c_device, O_RDWR);
    if (i2c_fd < 0) {
        perror("MPU6500: Failed to open I2C device");
        return 1;
    }
    
    if (ioctl(i2c_fd, I2C_SLAVE, current_i2c_addr) < 0) {
        perror("MPU6500: Failed to set I2C slave address");
        close(i2c_fd);
        i2c_fd = -1;
        return 1;
    }
    
    mpu6500_interface_debug_print("mpu6500: i2c init success on %s at 0x%02X.\n", 
                                   i2c_device, current_i2c_addr);
    return 0;
}

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t mpu6500_interface_iic_deinit(void)
{
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
    return 0;
}

/**
 * @brief      interface iic bus read
 * @param[in]  addr is the iic device write address
 * @param[in]  reg is the iic register address
 * @param[out] *buf points to a data buffer
 * @param[in]  len is the length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t mpu6500_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (i2c_fd < 0) {
        fprintf(stderr, "MPU6500: I2C not initialized\n");
        return 1;
    }

    // libdriver passes address with R/W bit, we need 7-bit address
    uint8_t i2c_addr = addr >> 1;
    
    // Set slave address if different from current
    if (i2c_addr != current_i2c_addr) {
        if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0) {
            perror("MPU6500: Failed to set I2C slave address for read");
            return 1;
        }
        current_i2c_addr = i2c_addr;
    }

    // Read data
    if (len == 1) {
        int32_t res = i2c_smbus_read_byte_data(i2c_fd, reg);
        if (res < 0) {
            perror("MPU6500: I2C read byte failed");
            return 1;
        }
        buf[0] = (uint8_t)res;
    } else {
        int res = i2c_smbus_read_i2c_block_data(i2c_fd, reg, len, buf);
        if (res != len) {
            perror("MPU6500: I2C block read failed");
            return 1;
        }
    }
    
    return 0;
}

/**
 * @brief     interface iic bus write
 * @param[in] addr is the iic device write address
 * @param[in] reg is the iic register address
 * @param[in] *buf points to a data buffer
 * @param[in] len is the length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t mpu6500_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (i2c_fd < 0) {
        fprintf(stderr, "MPU6500: I2C not initialized\n");
        return 1;
    }

    // libdriver passes address with R/W bit, we need 7-bit address
    uint8_t i2c_addr = addr >> 1;
    
    // Set slave address if different from current
    if (i2c_addr != current_i2c_addr) {
        if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0) {
            perror("MPU6500: Failed to set I2C slave address for write");
            return 1;
        }
        current_i2c_addr = i2c_addr;
    }

    // Write data
    if (len == 1) {
        if (i2c_smbus_write_byte_data(i2c_fd, reg, buf[0]) < 0) {
            perror("MPU6500: I2C write byte failed");
            return 1;
        }
    } else {
        if (i2c_smbus_write_i2c_block_data(i2c_fd, reg, len, buf) < 0) {
            perror("MPU6500: I2C block write failed");
            return 1;
        }
    }
    
    return 0;
}

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t mpu6500_interface_spi_init(void)
{
    return 1; // SPI not implemented
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t mpu6500_interface_spi_deinit(void)
{
    return 1; // SPI not implemented
}

/**
 * @brief      interface spi bus read
 * @param[in]  reg is the register address
 * @param[out] *buf points to a data buffer
 * @param[in]  len is the length of data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t mpu6500_interface_spi_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
    (void)reg;
    (void)buf;
    (void)len;
    return 1; // SPI not implemented
}

/**
 * @brief     interface spi bus write
 * @param[in] reg is the register address
 * @param[in] *buf points to a data buffer
 * @param[in] len is the length of data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t mpu6500_interface_spi_write(uint8_t reg, uint8_t *buf, uint16_t len)
{
    (void)reg;
    (void)buf;
    (void)len;
    return 1; // SPI not implemented
}

/**
 * @brief     interface delay ms
 * @param[in] ms
 * @note      none
 */
void mpu6500_interface_delay_ms(uint32_t ms)
{
    usleep(ms * 1000);
}

/**
 * @brief     interface print format data
 * @param[in] fmt is the format data
 * @note      none
 */
void mpu6500_interface_debug_print(const char *const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

/**
 * @brief     interface receive callback
 * @param[in] type is the irq type
 * @note      none
 */
void mpu6500_interface_receive_callback(uint8_t type)
{
    switch (type)
    {
        case MPU6500_INTERRUPT_MOTION :
        {
            mpu6500_interface_debug_print("mpu6500: irq motion.\n");
            break;
        }
        case MPU6500_INTERRUPT_FIFO_OVERFLOW :
        {
            mpu6500_interface_debug_print("mpu6500: irq fifo overflow.\n");
            break;
        }
        case MPU6500_INTERRUPT_DMP :
        {
            mpu6500_interface_debug_print("mpu6500: irq dmp.\n");
            break;
        }
        case MPU6500_INTERRUPT_DATA_READY :
        {
            mpu6500_interface_debug_print("mpu6500: irq data ready.\n");
            break;
        }
        default :
        {
            mpu6500_interface_debug_print("mpu6500: irq unknown code.\n");
            break;
        }
    }
}

/**
 * @brief     interface dmp tap callback
 * @param[in] count is the tap count
 * @param[in] direction is the tap direction
 * @note      none
 */
void mpu6500_interface_dmp_tap_callback(uint8_t count, uint8_t direction)
{
    switch (direction)
    {
        case MPU6500_DMP_TAP_X_UP :
        {
            mpu6500_interface_debug_print("mpu6500: tap irq x up with %d.\n", count);
            break;
        }
        case MPU6500_DMP_TAP_X_DOWN :
        {
            mpu6500_interface_debug_print("mpu6500: tap irq x down with %d.\n", count);
            break;
        }
        case MPU6500_DMP_TAP_Y_UP :
        {
            mpu6500_interface_debug_print("mpu6500: tap irq y up with %d.\n", count);
            break;
        }
        case MPU6500_DMP_TAP_Y_DOWN :
        {
            mpu6500_interface_debug_print("mpu6500: tap irq y down with %d.\n", count);
            break;
        }
        case MPU6500_DMP_TAP_Z_UP :
        {
            mpu6500_interface_debug_print("mpu6500: tap irq z up with %d.\n", count);
            break;
        }
        case MPU6500_DMP_TAP_Z_DOWN :
        {
            mpu6500_interface_debug_print("mpu6500: tap irq z down with %d.\n", count);
            break;
        }
        default :
        {
            mpu6500_interface_debug_print("mpu6500: tap irq unknown code.\n");
            break;
        }
    }
}

/**
 * @brief     interface dmp orient callback
 * @param[in] orientation is the dmp orientation
 * @note      none
 */
void mpu6500_interface_dmp_orient_callback(uint8_t orientation)
{
    switch (orientation)
    {
        case MPU6500_DMP_ORIENT_PORTRAIT :
        {
            mpu6500_interface_debug_print("mpu6500: orient irq portrait.\n");
            break;
        }
        case MPU6500_DMP_ORIENT_LANDSCAPE :
        {
            mpu6500_interface_debug_print("mpu6500: orient irq landscape.\n");
            break;
        }
        case MPU6500_DMP_ORIENT_REVERSE_PORTRAIT :
        {
            mpu6500_interface_debug_print("mpu6500: orient irq reverse portrait.\n");
            break;
        }
        case MPU6500_DMP_ORIENT_REVERSE_LANDSCAPE :
        {
            mpu6500_interface_debug_print("mpu6500: orient irq reverse landscape.\n");
            break;
        }
        default :
        {
            mpu6500_interface_debug_print("mpu6500: orient irq unknown code.\n");
            break;
        }
    }
}