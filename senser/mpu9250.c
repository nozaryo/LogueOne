
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "mpu9250.h"

#define MPU9250_ADDRESS 0x68
#define AK8963_ADDRESS 0x0C

#define MPU9250_PWR_MGMT_1 0x6B
#define MPU9250_PWR_MGMT_2 0x6C
#define MPU9250_SELF_TEST 0x00
#define MPU9250_CONFIG 0x1A
#define MPU9250_GYRO_CONFIG 0x1B
#define MPU9250_ACCEL_CONFIG 0x1C
#define MPU9250_ID 0x75
#define MPU9250_ACCEL_XOUT_H 0x3B
#define MPU9250_TEMP_OUT_H 0x41
#define MPU9250_GYRO_XOUT_H 0x67
 
#define MPU9250_WHOAMI 0x71


double *mpu9250_ax = NULL, *mpu9250_ay = NULL, *mpu9250_az = NULL; 
double *mpu9250_temp = NULL; 
double *mpu9250_gx = NULL, *mpu9250_gy = NULL, *mpu9250_gz = NULL; 
double *ak8963_tx = NULL, *ak8963_ty = NULL, *ak8963_tz = NULL; 
double accRange;
int gyroRange;

uint8_t mpu9250_data[20];
                  
int mpu9250_init(double *ax, double *ay, double *az, 
                double *temp,
                double *gx, double *gy, double *gz, 
                double *tx, double *ty, double *tz){
    for(int i = 0; i < 9; i++){
        mpu9250_data[i] = 0;
    }
    mpu9250_ax = ax;  mpu9250_ay = ay;  mpu9250_az = az; 
    mpu9250_temp = temp;
    mpu9250_gx = gx;  mpu9250_gy = gy;  mpu9250_gz = gz; 
    ak8963_tx = tx;  ak8963_ty = ty;  ak8963_tz = tz; 

    if(i2c_readSet(MPU9250_ADDRESS, MPU9250_ID) != MPU9250_WHOAMI){
        return 1;
    }

    i2c_writeSet(MPU9250_ADDRESS, MPU9250_PWR_MGMT_1, 0b10000000);
    i2c_writeSet(MPU9250_ADDRESS, MPU9250_PWR_MGMT_1, 0b00000000);
    i2c_writeSet(MPU9250_ADDRESS, MPU9250_ACCEL_CONFIG, 0b00011000);
    accRange = 16.0;
    i2c_writeSet(MPU9250_ADDRESS, MPU9250_GYRO_CONFIG, 0b00011000);
    gyroRange = 2000;
    return 0;
}

int mpu9250_getAll(){
    i2c_readSet_Nbyte(MPU9250_ADDRESS, &mpu9250_data[0], MPU9250_ACCEL_XOUT_H, 14);
    mpu9250_calcAcc();
    mpu9250_calcGyro();
    mpu9250_calcTemp();
    return 0;
}

int mpu9250_getAcc(){
    i2c_readSet_Nbyte(MPU9250_ADDRESS, &mpu9250_data[0], MPU9250_ACCEL_XOUT_H, 6);
    mpu9250_calcAcc();
    return 0;
}

int mpu9250_getGyro(){
    i2c_readSet_Nbyte(MPU9250_ADDRESS, &mpu9250_data[8], MPU9250_GYRO_XOUT_H, 6);
    mpu9250_calcGyro();
    return 0;
}

int mpu9250_getTemp(){
    i2c_readSet_Nbyte(MPU9250_ADDRESS, &mpu9250_data[6], MPU9250_TEMP_OUT_H, 2);
    mpu9250_calcTemp();
    return 0;
}

int mpu9250_calcAcc(){
    int16_t raw_ax, raw_ay, raw_az;
    raw_ax = (((int16_t)mpu9250_data[0]) << 8) | (int16_t)mpu9250_data[1] ;
    raw_ay = (((int16_t)mpu9250_data[2]) << 8) | (int16_t)mpu9250_data[3] ;
    raw_az = (((int16_t)mpu9250_data[4]) << 8) | (int16_t)mpu9250_data[5] ;

    *mpu9250_ax = raw_ax * accRange / 32768.0;
    *mpu9250_ay = raw_ay * accRange / 32768.0;
    *mpu9250_az = raw_az * accRange / 32768.0;

    return 0;

}

int mpu9250_calcGyro(){
    
}

int mpu9250_calcTemp(){

}
