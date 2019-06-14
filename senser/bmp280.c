
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"
#include "bmp280.h"


#define BMP280_ADDRESS 0x76

#define BMP280_TEMP_XLSB 0xFC
#define BMP280_TEMP_LSB 0xFB
#define BMP280_TEMP_MSB 0xFA
#define BMP280_PRESS_XLSB 0xF9
#define BMP280_PRESS_LSB 0xF8
#define BMP280_PRESS_MSB 0xF7
#define BMP280_CONFIG 0xF5
#define BMP280_CTRL_MEAS 0xF4
#define BMP280_STATUS 0xF3
#define BMP280_RESET 0xE0
#define BMP280_ID 0xD0
#define BMP280_CALIB 0x88//0x88 ~ 0x9F

#define BMP280_WHOAMI 0x58


int32_t t_fine;
double *bmp280_preAddress = NULL, *bmp280_tempAddress = NULL;
uint8_t bmp280_data[30]; //dig 24 data 6
/*
0 ~ 2   PressData
3 ~ 5   TempData
6 ~ 11  dig_TData
12 ~ 29 dig_PData
*/

int bmp280_init(double *a, double *b){
//return 0 正常終了
//return 1 異常終了
  for(int i = 0; i < 30; i++){
        bmp280_data[i] = 0;
  }
  bmp280_preAddress = a;
  bmp280_tempAddress = b;
  
  if(i2c_readSet(BMP280_ADDRESS, BMP280_ID) != BMP280_WHOAMI){
    return 1;
  }
  i2c_writeSet(BMP280_ADDRESS, BMP280_RESET, 0xB6);
  i2c_writeSet(BMP280_ADDRESS, BMP280_CTRL_MEAS, 0b00100111);// temp x16 pre x16 nomalMode
  i2c_writeSet(BMP280_ADDRESS, BMP280_CONFIG, 0b00010100);//tsb 0.5ms filter OFF
  return 0;

}

int bmp280_interrupt(int getData_quantity/*得られたデータ数*/){

    switch(TWSR & 0xF8){
        case 0x08://開始条件送信
            TWDR = (BMP280_ADDRESS<<1) + 0;
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
            break;

        case 0x18://SLA+W送信 ACK受信A
            if(0 <= getData_quantity && getData_quantity < 6){
                TWDR = BMP280_TEMP_MSB;
                TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
            }else if(6 <= getData_quantity && getData_quantity < 30){
                TWDR = BMP280_TEMP_MSB;
                TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
            }else{
                return 0;
            }
            break;
        
        case 0x28://データバイト送信 ACK受信
            TWCR = (1<<TWSTA)|(1<<TWINT)|(1<<TWEN)|(1<<TWIE);
            break;
        
        case 0x10://再送開始条件送信
            TWDR = (BMP280_ADDRESS<<1) + 1;
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);
            break;
        
        case 0x40://SLA+R送信 ACK受信
            if(getData_quantity == 5 || getData_quantity == 29){
                TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE);//NACK返答
            }else{
                TWCR = (1<<TWINT)|(1 << TWEA)|(1<<TWEN)|(1<<TWIE);//ACK返答
            }
            break;
        
        case 0x50://データバイト受信 ACK応答
            bmp280_data[getData_quantity] = TWDR;
            TWCR = (1<<TWINT)|(1 << TWEA)|(1<<TWEN)|(1<<TWIE);//ACK返答
            return getData_quantity + 1;
            break;
        
        case 0x58://データバイト受信 NACK応答
            bmp280_data[getData_quantity] = TWDR;
            TWCR = ((1<<TWSTO)|1<<TWINT)|(1<<TWEN)|(1<<TWIE);
            if(getData_quantity == 5){
                return getData_quantity + 1;
            }else if(getData_quantity == 29){
                bmp280_calcPre();
                bmp280_calcTemp();
                return 0;
            }else{
                return -1;
            }
            break;
        
        default:
            return -1;
            break;
    }
    return -1;
}

int  bmp280_getPre(){

//    i2c_writeSet(BMP280_ADDRESS, BMP280_CTRL_MEAS, 0b00101111);// temp x1 pre x4 nomalMode
    _delay_ms(1);
    if(bmp280_getTemp()) return 1;
    i2c_readSet_Nbyte(BMP280_ADDRESS,&bmp280_data[0] ,BMP280_PRESS_MSB, 3);
    i2c_readSet_Nbyte(BMP280_ADDRESS,&bmp280_data[12] ,BMP280_CALIB + 6, 18);
    if(bmp280_calcPre()) return 1;
    return 0;

}


int  bmp280_getTemp(){

//    i2c_writeSet(BMP280_ADDRESS, BMP280_CTRL_MEAS, 0b00101111);// temp x1 pre x4 nomalMode
    _delay_ms(1);
    i2c_readSet_Nbyte(BMP280_ADDRESS,&bmp280_data[3] ,BMP280_TEMP_MSB, 3);
    i2c_readSet_Nbyte(BMP280_ADDRESS,&bmp280_data[6] ,BMP280_CALIB , 6);
    if(bmp280_calcTemp()) return 1;
    return 0;


}

int bmp280_calcPre(){

    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint32_t adc_P;
    int64_t var1, var2, p;

    if(bmp280_preAddress == NULL){
        return 1;
    }

    dig_P1 = (uint16_t)bmp280_data[12] | ((uint16_t)bmp280_data[13] << 8);
    dig_P2 = (int16_t)bmp280_data[14] | ((int16_t)bmp280_data[15] << 8);
    dig_P3 = (int16_t)bmp280_data[16] | ((int16_t)bmp280_data[17] << 8);
    dig_P4 = (int16_t)bmp280_data[18] | ((int16_t)bmp280_data[19] << 8);
    dig_P5 = (int16_t)bmp280_data[20] | ((int16_t)bmp280_data[21] << 8);
    dig_P6 = (int16_t)bmp280_data[22] | ((int16_t)bmp280_data[23] << 8);
    dig_P7 = (int16_t)bmp280_data[24] | ((int16_t)bmp280_data[24] << 8);
    dig_P8 = (int16_t)bmp280_data[26] | ((int16_t)bmp280_data[26] << 8);
    dig_P9 = (int16_t)bmp280_data[28] | ((int16_t)bmp280_data[29] << 8);
    adc_P = ((uint32_t)bmp280_data[0] << 12) | ((uint32_t)bmp280_data[1] << 4) | (((uint32_t)bmp280_data[2] >> 4 ) & 0xf );

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    if (var1 == 0) {
        return 1 ; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    *bmp280_preAddress  = (double)( ((uint32_t)p) / 256.0 / 100.0 );
    return 0;
}

int bmp280_calcTemp(){

    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint32_t adc_T;
    int32_t var1, var2, T;

    if(bmp280_tempAddress == NULL){
        return 1;
    }

    dig_T1 = (uint16_t)bmp280_data[6] | ((uint16_t)bmp280_data[7] << 8);
    dig_T2 = (int16_t)bmp280_data[8] | ((int16_t)bmp280_data[9] << 8);
    dig_T3 = (int16_t)bmp280_data[10] | ((int16_t)bmp280_data[11] << 8);
    adc_T = ((uint32_t)bmp280_data[3] << 12) | ((uint32_t)bmp280_data[4] << 4) | (((uint32_t)bmp280_data[5] >> 4) & 0xf );

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    *bmp280_tempAddress = (double)(T / 100.0);
    return 0;
}
