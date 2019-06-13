
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


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

void i2c_write(int data){
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & (1<<TWINT)));
    return;
}

void i2c_writeSet(uint8_t addres, uint8_t registerID, uint8_t writeData){

    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));
    i2c_write((addres << 1) + 0);
    i2c_write(registerID);
    i2c_write(writeData);
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
    _delay_ms(10);
    return;
}

uint8_t i2c_readSet(uint8_t addres, uint8_t registerID){
    
    uint8_t readData = 0;
    
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));
    TWDR = (addres<<1) + 0;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));
    TWDR = registerID;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));

    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));
    TWDR = (addres<<1) + 1;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));
    readData = TWDR;
    TWCR = ((1<<TWSTO)|1<<TWINT)|(1<<TWEN);

    return readData;
}

int i2c_readSet_Nbyte
(uint8_t addres, uint8_t* getData, uint8_t firstRegister, int registerQty){
    
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));
    TWDR = (addres<<1) + 0;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));
    TWDR = firstRegister;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));

    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));
    TWDR = (addres<<1) + 1;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while(!(TWCR & 1<<TWINT));

    for(int i = 0; i < registerQty; i++){
        if(i == registerQty - 1){
            TWCR = (1<<TWINT)|(1<<TWEN);//NACK返答
        }else{
            TWCR = (1<<TWINT)|(1 << TWEA)|(1<<TWEN)|(1<<TWIE);//ACK返答
        }
        while(!(TWCR & 1<<TWINT));
        readData = TWDR;
    }
    
    TWCR = ((1<<TWSTO)|1<<TWINT)|(1<<TWEN);

}

void i2c_stop(){
    TWCR = ((1<<TWSTO)|1<<TWINT)|(1<<TWEN)|(1<<TWIE);
}

int i2c_wait(){

    while(!(TWCR & 1<<TWINT)){
    
    }
}
