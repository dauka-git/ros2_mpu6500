#include "ros2_mpu6500/mpu6500.h"
#include <iostream>
#include <cmath>
#include <cstring>

extern "C" {
#include "driver_mpu6500.h"
#include "driver_mpu6500_interface.h"
}

Mpu6500::Mpu6500(const std::string &device, int i2c_address)
    : initialized_(false)
{
    (void)device; // Device path is set in mpu6500_interface.c
    
    std::cout << "Initializing MPU6500 at address 0x" << std::hex << i2c_address << std::dec << std::endl;
    
    // Link interface functions
    DRIVER_MPU6500_LINK_INIT(&handle_, mpu6500_handle_t);
    DRIVER_MPU6500_LINK_IIC_INIT(&handle_, mpu6500_interface_iic_init);
    DRIVER_MPU6500_LINK_IIC_DEINIT(&handle_, mpu6500_interface_iic_deinit);
    DRIVER_MPU6500_LINK_IIC_READ(&handle_, mpu6500_interface_iic_read);
    DRIVER_MPU6500_LINK_IIC_WRITE(&handle_, mpu6500_interface_iic_write);
    DRIVER_MPU6500_LINK_SPI_INIT(&handle_, mpu6500_interface_spi_init);
    DRIVER_MPU6500_LINK_SPI_DEINIT(&handle_, mpu6500_interface_spi_deinit);
    DRIVER_MPU6500_LINK_SPI_READ(&handle_, mpu6500_interface_spi_read);
    DRIVER_MPU6500_LINK_SPI_WRITE(&handle_, mpu6500_interface_spi_write);
    DRIVER_MPU6500_LINK_DELAY_MS(&handle_, mpu6500_interface_delay_ms);
    DRIVER_MPU6500_LINK_DEBUG_PRINT(&handle_, mpu6500_interface_debug_print);
    DRIVER_MPU6500_LINK_RECEIVE_CALLBACK(&handle_, mpu6500_interface_receive_callback);

    // Set interface to I2C
    if (mpu6500_set_interface(&handle_, MPU6500_INTERFACE_IIC) != 0) {
        std::cerr << "Failed to set I2C interface" << std::endl;
        throw std::runtime_error("MPU6500: Failed to set interface");
    }

    // Set I2C address based on AD0 pin
    mpu6500_address_t addr = (i2c_address == MPU6500_ADDRESS_AD0_LOW) ? 
                              MPU6500_ADDRESS_AD0_LOW : MPU6500_ADDRESS_AD0_HIGH;
    if (mpu6500_set_addr_pin(&handle_, addr) != 0) {
        std::cerr << "Failed to set address pin" << std::endl;
        throw std::runtime_error("MPU6500: Failed to set address");
    }

    // Initialize the device
    if (mpu6500_init(&handle_) != 0) {
        std::cerr << "Failed to initialize MPU6500 device" << std::endl;
        throw std::runtime_error("MPU6500: Initialization failed");
    }

    // Configure gyroscope - ±2000°/s range
    if (mpu6500_set_gyroscope_range(&handle_, MPU6500_GYROSCOPE_RANGE_2000DPS) != 0) {
        std::cerr << "Failed to set gyroscope range" << std::endl;
        mpu6500_deinit(&handle_);
        throw std::runtime_error("MPU6500: Failed to configure gyroscope");
    }

    // Configure accelerometer - ±8g range
    if (mpu6500_set_accelerometer_range(&handle_, MPU6500_ACCELEROMETER_RANGE_8G) != 0) {
        std::cerr << "Failed to set accelerometer range" << std::endl;
        mpu6500_deinit(&handle_);
        throw std::runtime_error("MPU6500: Failed to configure accelerometer");
    }

    // Set clock source to PLL
    if (mpu6500_set_clock_source(&handle_, MPU6500_CLOCK_SOURCE_PLL) != 0) {
        std::cerr << "Failed to set clock source" << std::endl;
        mpu6500_deinit(&handle_);
        throw std::runtime_error("MPU6500: Failed to set clock");
    }

    // Enable low pass filter - 92Hz bandwidth
    if (mpu6500_set_low_pass_filter(&handle_, MPU6500_LOW_PASS_FILTER_2) != 0) {
        std::cerr << "Failed to set low pass filter" << std::endl;
        mpu6500_deinit(&handle_);
        throw std::runtime_error("MPU6500: Failed to set filter");
    }

    // Set sample rate divider (1kHz / (1 + 0) = 1kHz)
    if (mpu6500_set_sample_rate_divider(&handle_, 0) != 0) {
        std::cerr << "Failed to set sample rate divider" << std::endl;
        mpu6500_deinit(&handle_);
        throw std::runtime_error("MPU6500: Failed to set sample rate");
    }

    // Disable FIFO - we'll read directly from registers instead
    if (mpu6500_set_fifo(&handle_, MPU6500_BOOL_FALSE) != 0) {
        std::cerr << "Warning: Failed to disable FIFO" << std::endl;
    }

    std::cout << "MPU6500 initialized successfully!" << std::endl;
    std::cout << "  - Gyroscope range: ±2000°/s" << std::endl;
    std::cout << "  - Accelerometer range: ±8g" << std::endl;
    std::cout << "  - Low-pass filter: 92Hz" << std::endl;
    std::cout << "  - Sample rate: 1kHz" << std::endl;
    std::cout << "  - Reading directly from registers" << std::endl;
    
    initialized_ = true;
}

Mpu6500::~Mpu6500()
{
    if (initialized_) {
        mpu6500_deinit(&handle_);
        std::cout << "MPU6500 deinitialized" << std::endl;
    }
}

Mpu6500Hal::Mpu6500_Error_t Mpu6500::Mpu6500_GetAccelData(Mpu6500_AccelData_t &AccelData)
{
    if (!initialized_) {
        std::cerr << "MPU6500 not initialized" << std::endl;
        return Mpu6500Hal::MPU6500_ERR;
    }

    // Read directly from registers using libdriver's read_accel function
    int16_t accel_raw[3][1];
    float accel_g[3][1];
    
    uint8_t res = mpu6500_read_accel(&handle_, accel_raw, accel_g);
    if (res != 0) {
        std::cerr << "Failed to read accelerometer" << std::endl;
        return Mpu6500Hal::MPU6500_ERR;
    }

    // Convert g to m/s² (1g = 9.80665 m/s²)
    const double G_TO_MS2 = 9.80665;
    AccelData.Accel_X = accel_g[0][0] * G_TO_MS2;
    AccelData.Accel_Y = accel_g[1][0] * G_TO_MS2;
    AccelData.Accel_Z = accel_g[2][0] * G_TO_MS2;

    return Mpu6500Hal::MPU6500_OK;
}

Mpu6500Hal::Mpu6500_Error_t Mpu6500::Mpu6500_GetGyroData(Mpu6500_GyroData_t &GyroData)
{
    if (!initialized_) {
        std::cerr << "MPU6500 not initialized" << std::endl;
        return Mpu6500Hal::MPU6500_ERR;
    }

    // Read directly from registers using libdriver's read_gyro function
    int16_t gyro_raw[3][1];
    float gyro_dps[3][1];
    
    uint8_t res = mpu6500_read_gyro(&handle_, gyro_raw, gyro_dps);
    if (res != 0) {
        std::cerr << "Failed to read gyroscope" << std::endl;
        return Mpu6500Hal::MPU6500_ERR;
    }

    // Convert degrees per second to radians per second
    const double DEG_TO_RAD = M_PI / 180.0;
    GyroData.Gyro_X = gyro_dps[0][0] * DEG_TO_RAD;
    GyroData.Gyro_Y = gyro_dps[1][0] * DEG_TO_RAD;
    GyroData.Gyro_Z = gyro_dps[2][0] * DEG_TO_RAD;

    return Mpu6500Hal::MPU6500_OK;
}