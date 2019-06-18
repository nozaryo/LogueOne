
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint64_t i2c_timer;
void i2c_init(){
//i2c setting
    TWBR = 2;//400kHz
    TWSR = 0b00000010;
    TWCR = (1<<TWEN)|(1<<TWIE);
}

void i2c_start(){
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN)|(1<<TWIE);
    return;
}

int i2c_write(int data){
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if(i2c_wait()) return 1;
    return 0;
}

int i2c_writeSet(uint8_t addres, uint8_t registerID, uint8_t writeData){

    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    if(i2c_wait()) return 1;
    i2c_write((addres << 1) + 0);
    i2c_write(registerID);
    i2c_write(writeData);
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
    _delay_ms(10);
    return 0;
}

uint8_t i2c_readSet(uint8_t addres, uint8_t registerID){
    
    uint8_t readData = 0;
    
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    if(i2c_wait()) return 0x00;
    TWDR = (addres<<1) + 0;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if(i2c_wait()) return 0x00;
    TWDR = registerID;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if(i2c_wait()) return 0x00;

    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    if(i2c_wait()) return 0x00;
    TWDR = (addres<<1) + 1;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if(i2c_wait()) return 0x00;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if(i2c_wait()) return 0x00;
    readData = TWDR;
    TWCR = ((1<<TWSTO)|1<<TWINT)|(1<<TWEN);

    return readData;
}

int i2c_readSet_Nbyte
(uint8_t addres, uint8_t* getData, uint8_t firstRegister, int registerQty){
    
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    if(i2c_wait()) return 1;
    TWDR = (addres<<1) + 0;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if(i2c_wait()) return 1;
    TWDR = firstRegister;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if(i2c_wait()) return 1;

    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    if(i2c_wait()) return 1;
    TWDR = (addres<<1) + 1;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if(i2c_wait()) return 1;

    for(int i = 0; i < registerQty; i++){
        if(i == registerQty - 1){
            TWCR = (1<<TWINT)|(1<<TWEN);//NACK返答
        }else{
            TWCR = (1<<TWINT)|(1 << TWEA)|(1<<TWEN)|(1<<TWIE);//ACK返答
        }
        if(i2c_wait()) return 1;
        getData[i] = TWDR;
    }
    
    TWCR = ((1<<TWSTO)|1<<TWINT)|(1<<TWEN);
    return 0;
}

void i2c_stop(){
    TWCR = ((1<<TWSTO)|1<<TWINT)|(1<<TWEN)|(1<<TWIE);
}

int i2c_wait(){
    uint64_t nowtime = i2c_timer;
    while(!(TWCR & 1<<TWINT)){
        if(i2c_timer - nowtime > 10){
            return 1;            
        }
    }
    return 0;
}
