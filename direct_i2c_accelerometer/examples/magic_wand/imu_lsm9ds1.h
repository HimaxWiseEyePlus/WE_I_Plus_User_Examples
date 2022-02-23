#ifndef INC_IMU_LSM9DS1_H_
#define INC_IMU_LSM9DS1_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HX_DRV_IMU_PASS = 0,
    HX_DRV_IMU_ERROR = -1,
	HX_DRV_IMU_MORE_DATA = -2,
	HX_DRV_IMU_NO_DATA = -3,
} HX_DRV_IMU_ERROR_E;

HX_DRV_IMU_ERROR_E imu_initial();
HX_DRV_IMU_ERROR_E imu_receive(float *x, float *y, float *z);
HX_DRV_IMU_ERROR_E imu_accelerationAvailable();
uint8_t imu_accelerationAvailableCount();
#ifdef __cplusplus
}
#endif


#endif /* INC_IMU_LSM9DS1_H_ */
