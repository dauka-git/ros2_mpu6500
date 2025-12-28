#ifndef MPU6500SENSOR_H
#define MPU6500SENSOR_H

#include "ros2_mpu6500/mpu6500_hal.h"

extern "C" {
#include "driver_mpu6500.h"
}

#include <string>
#include <memory>
#include <cstdint>

class Mpu6500 {
public:
    explicit Mpu6500(const std::string &device = "/dev/i2c-7", int i2c_address = MPU6500_ADDRESS_AD0_LOW);

    ~Mpu6500();

    /**
     * @brief Accelerometer data
     */
    typedef struct{
        double Accel_X;     /** @brief Accel X data in m/s² */
        double Accel_Y;     /** @brief Accel Y data in m/s² */
        double Accel_Z;     /** @brief Accel Z data in m/s² */
    } Mpu6500_AccelData_t;

    /**
     * @brief Gyroscope data
     */
    typedef struct{
        double Gyro_X;     /** @brief Gyro X data in rad/s */
        double Gyro_Y;     /** @brief Gyro Y data in rad/s */
        double Gyro_Z;     /** @brief Gyro Z data in rad/s */
    } Mpu6500_GyroData_t;

    /**
    * @brief        Get accelerometer data.
    * @details      Get accelerometer data in m/s².
    *
    * @param[out]   AccelData   Accelerometer data.
    *
    * @return       Mpu6500Hal::Mpu6500_Error_t     Return code.
    *
    */
    Mpu6500Hal::Mpu6500_Error_t Mpu6500_GetAccelData(Mpu6500_AccelData_t &AccelData);

    /**
    * @brief        Get gyroscope data.
    * @details      Get gyroscope data in rad/s.
    *
    * @param[out]   GyroData    Gyroscope data.
    *
    * @return       Mpu6500Hal::Mpu6500_Error_t     Return code.
    *
    */
    Mpu6500Hal::Mpu6500_Error_t Mpu6500_GetGyroData(Mpu6500_GyroData_t &GyroData);

private:
    mpu6500_handle_t handle_;
    bool initialized_;
};

#endif  // MPU6500SENSOR_H