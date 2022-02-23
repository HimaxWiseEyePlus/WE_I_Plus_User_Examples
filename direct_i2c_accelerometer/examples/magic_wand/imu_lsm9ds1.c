#include <stdio.h>
#include "hx_drv_tflm.h"
#include "imu_lsm9ds1.h"


#define HX_IMU_I2C_ADDR_BYTE    1               /**< I2C Master Register address length*/
#define HX_IMU_I2C_DATA_BYTE    1               /**< I2C Master Register value length*/

#define HX_IMU_I2C_RETRY_TIME  3  /**< If I2C Master set fail, maximum retry time for imu setting*/

/** I2C Device Address 7 bit format **/
#define LSM9DS1_IMU_I2C_ADDR    0x6A            // 7 bit format
//#define LSM9DS1_IMU_I2C_ADDR    0x6B            // 7 bit format
#define LSM9DS1_IMU_ID          0x68            // Device Identification (Who am I)
#define LSM9DS1_WHO_AM_I        0x0F
#define LSM9DS1_CTRL_REG1_G     0x10
#define LSM9DS1_STATUS_REG      0x17
#define LSM9DS1_OUT_X_G         0x18
#define LSM9DS1_CTRL_REG6_XL    0x20
#define LSM9DS1_CTRL_REG8       0x22
#define LSM9DS1_OUT_X_XL        0x28
#define LSM9DS1_FIFO_SRC        0x2F

// magnetometer
#define LSM9DS1_MAG_I2C_ADDR    0x1C            // 7 bit format
//#define LSM9DS1_MAG_I2C_ADDR    0x1E            // 7 bit format
#define LSM9DS1_MAG_ID          0x3D            // Device Identification (Who am I)
#define LSM9DS1_CTRL_REG1_M     0x20
#define LSM9DS1_CTRL_REG2_M     0x21
#define LSM9DS1_CTRL_REG3_M     0x22
#define LSM9DS1_STATUS_REG_M    0x27
#define LSM9DS1_OUT_X_L_M       0x28

#define MS_TCIK                 400000          //CPU run at 400Mhz, 400000 ticks is 1 ms

static uint8_t g_continuousMode = 0;

HX_DRV_IMU_ERROR_E imu_set_reg(uint8_t slv_addr, uint8_t addr, uint8_t val)
{
    uint8_t regAddr[HX_IMU_I2C_ADDR_BYTE]  = {addr};
    uint8_t wBuffer[HX_IMU_I2C_DATA_BYTE] = {val};
    int32_t retI2C = 0;

    for(int i = 0;i<HX_IMU_I2C_RETRY_TIME;i++) {
      retI2C = hx_drv_acc_i2cm_set_data(slv_addr, regAddr, HX_IMU_I2C_ADDR_BYTE, wBuffer, HX_IMU_I2C_DATA_BYTE);
      if(retI2C == HX_DRV_LIB_PASS)
        return HX_DRV_IMU_PASS;
    }

    return HX_DRV_IMU_ERROR;
}

HX_DRV_IMU_ERROR_E imu_get_reg(uint8_t slv_addr, uint8_t addr, uint8_t *val)
{
    uint8_t regAddr[HX_IMU_I2C_ADDR_BYTE]  = {addr};
    uint8_t rBuffer[HX_IMU_I2C_DATA_BYTE] = {0x00};
    int32_t retI2C = 0;

    *val = 0;

    retI2C = hx_drv_acc_i2cm_get_data(slv_addr, regAddr, HX_IMU_I2C_ADDR_BYTE, rBuffer, HX_IMU_I2C_DATA_BYTE);
    if(retI2C != HX_DRV_LIB_PASS)
    {
        return HX_DRV_IMU_ERROR;
    }

   	*val = rBuffer[0];

    return HX_DRV_IMU_PASS;
}

void imu_setContinuousMode()
{
    // Enable FIFO 
    imu_set_reg(LSM9DS1_IMU_I2C_ADDR, 0x23, 0x02);
    
    // Set continuous mode
    imu_set_reg(LSM9DS1_IMU_I2C_ADDR, 0x2E, 0xC0);

    g_continuousMode = 1;
}

void imu_setOneShotMode()
{
    // Disable FIFO 
    imu_set_reg(LSM9DS1_IMU_I2C_ADDR, 0x23, 0x00);

    // Disable continuous mode
    imu_set_reg(LSM9DS1_IMU_I2C_ADDR, 0x2E, 0x00);
    g_continuousMode = 0;
}

float imu_accelerationSampleRate()
{
    return 119.0F;
}

uint8_t imu_accelerationAvailableCount()
{
    if (g_continuousMode)
    {
        // g_continuousMode: Enable FIFO
        // Read FIFO_SRC. If any of the rightmost 8 bits have a value, there is data.
        uint8_t un_read_num = 0;
        if(imu_get_reg(LSM9DS1_IMU_I2C_ADDR, LSM9DS1_FIFO_SRC, &un_read_num) == HX_DRV_IMU_ERROR)
            return 0;
        
        return un_read_num& 0x3F;
    }

    return 0;
}

HX_DRV_IMU_ERROR_E imu_accelerationAvailable()
{
    if (g_continuousMode)
    {
        // g_continuousMode: Enable FIFO
        // Read FIFO_SRC. If any of the rightmost 8 bits have a value, there is data.
        uint8_t un_read_num = 0;
        if(imu_get_reg(LSM9DS1_IMU_I2C_ADDR, LSM9DS1_FIFO_SRC, &un_read_num) == HX_DRV_IMU_ERROR)
            return HX_DRV_IMU_ERROR;

        if(un_read_num!=0)
            return HX_DRV_IMU_MORE_DATA;
        else
          	return HX_DRV_IMU_NO_DATA;

    }
    else
    {
        // OneShotMode: Disable FIFO
        uint8_t valid = 0;
        if(imu_get_reg(LSM9DS1_IMU_I2C_ADDR, LSM9DS1_STATUS_REG, &valid) == HX_DRV_IMU_ERROR)
            return HX_DRV_IMU_ERROR;

        if(valid & 0x01)
            return HX_DRV_IMU_MORE_DATA;
        else
            return HX_DRV_IMU_NO_DATA;

    }

    return HX_DRV_IMU_PASS;
}


HX_DRV_IMU_ERROR_E imu_initial()   
{
    uint8_t dev_id = 0xFF;
    uint32_t tick_target=0, tick_cur=0;
    // reset
    imu_set_reg(LSM9DS1_IMU_I2C_ADDR, LSM9DS1_CTRL_REG8, 0x05);
    imu_set_reg(LSM9DS1_MAG_I2C_ADDR, LSM9DS1_CTRL_REG2_M, 0x0c);

    //delay 10ms
    hx_drv_tick_start();
    hx_drv_tick_get(&tick_target);
    hx_drv_tick_get(&tick_cur);
    tick_target += MS_TCIK*100;
    while(tick_cur<tick_target) {
        hx_drv_tick_get(&tick_cur);
    }

    if(imu_get_reg(LSM9DS1_IMU_I2C_ADDR, LSM9DS1_WHO_AM_I, &dev_id) == HX_DRV_IMU_ERROR)
    {
        return HX_DRV_IMU_ERROR;
    }
    else if(dev_id != LSM9DS1_IMU_ID)
        return HX_DRV_IMU_ERROR;
 
    if(imu_get_reg(LSM9DS1_MAG_I2C_ADDR, LSM9DS1_WHO_AM_I, &dev_id) == HX_DRV_IMU_ERROR)
        return HX_DRV_IMU_ERROR;
    else if(dev_id != LSM9DS1_MAG_ID)
        return HX_DRV_IMU_ERROR;

    imu_set_reg(LSM9DS1_IMU_I2C_ADDR, LSM9DS1_CTRL_REG1_G, 0x78);    // 119 Hz, 2000 dps, 16 Hz BW
    imu_set_reg(LSM9DS1_IMU_I2C_ADDR, LSM9DS1_CTRL_REG6_XL, 0x70);   // 119 Hz, 4G

    imu_set_reg(LSM9DS1_MAG_I2C_ADDR, LSM9DS1_CTRL_REG1_M, 0xb4);    // Temperature compensation enable, medium performance, 20 Hz
    imu_set_reg(LSM9DS1_MAG_I2C_ADDR, LSM9DS1_CTRL_REG2_M, 0x00);    // 4 Gauss
    imu_set_reg(LSM9DS1_MAG_I2C_ADDR, LSM9DS1_CTRL_REG3_M, 0x00);    // Continuous conversion mode

    imu_setContinuousMode();

    return HX_DRV_IMU_PASS;
}

HX_DRV_IMU_ERROR_E imu_receive(float *x, float *y, float *z)
{
    uint8_t regAddr[HX_IMU_I2C_ADDR_BYTE]  = {LSM9DS1_OUT_X_XL};
    uint8_t rBuffer[6] = {0x00};
    int32_t retI2C = 0;
    int16_t gx_raw, gy_raw, gz_raw;

    retI2C = hx_drv_acc_i2cm_get_data( LSM9DS1_IMU_I2C_ADDR, regAddr, HX_IMU_I2C_ADDR_BYTE, rBuffer, 6);
    if(retI2C != HX_DRV_LIB_PASS)
        return HX_DRV_IMU_ERROR;
    
    //SCALE_4G:
    gx_raw = rBuffer[0] | (rBuffer[1] << 8);
    gy_raw = rBuffer[2] | (rBuffer[3] << 8);
    gz_raw = rBuffer[4] | (rBuffer[5] << 8);

   	//*x = (float)gx_raw * 4.0 / 32768.0;
    //*y = (float)gy_raw * 4.0 / 32768.0;
    //*z = (float)gz_raw * 4.0 / 32768.0;
    *x = (float)gx_raw/8192.0;
    *y = (float)gy_raw/8192.0;
    *z = (float)gz_raw/8192.0;


    return HX_DRV_IMU_PASS;
}


