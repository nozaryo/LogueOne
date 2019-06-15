#ifndef _ADXL375_H_
#define _ADXL375_H_

extern uint8_t ADXL375_data[6];
extern double ADXL375_offsetX, ADXL375_offsetY, ADXL375_offsetZ;

void ADXL375_setOffset(double ox, double oy, double oz);
int ADXL375_init(double *x, double *y, double *z);
int ADXL375_interrupt(int callingCnt);
int ADXL375_getAcc();
int ADXL375_calcAcc();

#endif
