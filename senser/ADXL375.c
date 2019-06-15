
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "ADXL375.h"

#define ADXL375_ADDRESS 0x1D

#define ADXL375_DEVID 0x00
#define ADXL375_POWER_CTL 0x2D
#define ADXL375_DATA_FORMAT 0x31
#define ADXL375_DATAX0 0x32
#define ADXL375_DATAX1 0x33
#define ADXL375_DATAY0 0x34
#define ADXL375_DATAY1 0x35
#define ADXL375_DATAZ0 0x36
#define ADXL375_DATAZ1 0x37

#define ADXL375_WHOAMI 0xE5

double *ADXL375_accX = NULL, *ADXL375_accY = NULL, *ADXL375_accZ = NULL;
double ADXL375_offsetX = 0, ADXL375_offsetY = 0, ADXL375_offsetZ = 0;
uint8_t ADXL375_data[6];

int ADXL375_init(double *x, double *y, double *z){

    for(int i = 0; i < 6; i++){
        ADXL375_data[i] = 0;
    }
    
    if(i2c_readSet(ADXL375_ADDRESS, ADXL375_DEVID) != ADXL375_WHOAMI){
        return 1;
    }

    ADXL375_accX = x;
    ADXL375_accY = y;
    ADXL375_accZ = z;
    i2c_writeSet(ADXL375_ADDRESS, ADXL375_POWER_CTL, 0x08);
    i2c_writeSet(ADXL375_ADDRESS, ADXL375_DATA_FORMAT, 0x0B);
    return 0;

}

void ADXL375_setOffset(double ox, double oy, double oz){

    ADXL375_offsetX = ox;
    ADXL375_offsetY = oy;
    ADXL375_offsetZ = oz;

}

int ADXL375_interrupt(int callingCnt){

}

int ADXL375_getAcc(){
    i2c_readSet_Nbyte(ADXL375_ADDRESS, &ADXL375_data[0], ADXL375_DATAX0, 6);
    if(ADXL375_calcAcc()) return 1;
    return 0;
}

int ADXL375_calcAcc(){
    int16_t accX_raw, accY_raw, accZ_raw;

    accX_raw = ( (uint16_t)ADXL375_data[1] << 8) | ( (uint16_t)ADXL375_data[0] );
    accY_raw = ( (uint16_t)ADXL375_data[3] << 8) | ( (uint16_t)ADXL375_data[2] );
    accZ_raw = ( (uint16_t)ADXL375_data[5] << 8) | ( (uint16_t)ADXL375_data[4] );
    
    *ADXL375_accX = (((double)accX_raw) / 20.0f) - ADXL375_offsetX;
    *ADXL375_accY = (((double)accY_raw) / 20.0f) - ADXL375_offsetY;
    *ADXL375_accZ = (((double)accZ_raw) / 20.0f) - ADXL375_offsetZ;

    return 0;
}

