#ifndef _MPU9250_H_
#define _MPU9250_H_

extern double accRange;
extern int gyroRange;
extern uint8_t mpu9250_data[20];
int mpu9250_init(double *ax, double *ay, double *az, 
                double *temp,
                double *gx, double *gy, double *gz, 
                double *tx, double *ty, double *tz);
int mpu9250_getAll();
int mpu9250_getAcc();
int mpu9250_getGyro();
int mpu9250_getTemp();
int mpu9250_calcAcc();
int mpu9250_calcGyro();
int mpu9250_calcTemp();

#endif
