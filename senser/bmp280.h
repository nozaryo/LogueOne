#ifndef _BMP280_H_
#define _BMP280_H_

int bmp280_init(double* a, double* b);
int bmp280_interrupt(int getData_quantity/*得られたデータ数*/);
void  bmp280_getPre();
void  bmp280_getTemp();
void bmp280_calcPre();
void bmp280_calcTemp();

#endif // _BMP280_H_
