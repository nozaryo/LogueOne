#ifndef _I2C_H_
#define _I2C_H_

#define NULL 0

void i2c_init();
void i2c_start();
void i2c_write(int data);
void i2c_writeSet(uint8_t addres, uint8_t registerID, uint8_t writeData);
uint8_t i2c_readSet(uint8_t addres, uint8_t registerID);
int i2c_readSet_Nbyte
(uint8_t addres, uint8_t* getData, uint8_t firstRegister, int registerQty);
void i2c_stop();
int i2c_wait();

#endif // _I2C_H_
