
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c.h"

#define BMP280_ADDRESS 0x77

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


double* bmp280_preAddress = NULL, bmp280_tempAddress = NULL;
uint8_t bmp280_data[30]; //dig 24 data 6
/*
0 ~ 2   PressData
3 ~ 5   TempData
6 ~ 11  dig_TData
12 ~ 29 dig_PData
*/

int bmp280_init(double* a, double* b){
//return 0 正常終了
//return 1 異常終了
  bmp280_preAddress = a;
  bmp280_tempAddress = b;
  
  if(i2c_readSet(BMP280_ADDRESS, BMP280_ID) != BMP280_WHOAMI){
    return 1;
  }

  i2c_writeSet(BMP280_ADDRESS, BMP280_CTRL_MEAS, 0b10110111);//pre x16 temp x16 nomalMode
  i2c_writeSet(BMP280_ADDRESS, BMP280_CONFIG, 0b0000000);//tsb 0.5ms filter OFF
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

}

void  bmp280_getPre(){

    i2c_readSet_Nbyte(BMP280_ADDRESS,&bmp280_data[0] ,BMP280_PRESS_MSB, 3);
    i2c_readSet_Nbyte(BMP280_ADDRESS,&bmp280_data[12] ,BMP280_CALIB + 6, 18);
    bmp280_calcPre();

}


void  bmp280_getTemp(){

    i2c_readSet_Nbyte(BMP280_ADDRESS,&bmp280_data[3] ,BMP280_TEMP_MSB, 3);
    i2c_readSet_Nbyte(BMP280_ADDRESS,&bmp280_data[6] ,BMP280_CALIB , 6);
    bmp280_calcTemp();


}

void bmp280_calcPre(){
    if(bmp280_preAddress == NULL){
        return 0;
    }

}

void bmp280_calcTemp(){
    if(bmp280_tempAddress == NULL){
        return 0;
    }

}
