#ifndef _BMP280_H_
#define _BMP280_H_


extern int32_t t_fine;
extern uint8_t bmp280_data[30]; //dig 24 data 6
int bmp280_init(double* a, double* b);
int bmp280_interrupt(int getData_quantity/*得られたデータ数*/);
int bmp280_getAll();
int bmp280_getPre();
int bmp280_getTemp();
int bmp280_calcAll();
int bmp280_calcPre();
int bmp280_calcTemp();

#endif // _BMP280_H_
