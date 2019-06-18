
/* AtMega328p/@16MHz USART 9600BAUD							*/
/*															*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>
#include <stdio.h>
#include "senser/i2c.h"
#include "senser/bmp280.h"
#include "senser/ADXL375.h"
#include "senser/mpu9250.h"
#include "FatFs/ff.h"
#include "MMC/mmc.h"

#define FIFO_SIZE 16				/* FIFO buffer size		*/
#define BAUD 9600					/* USART BAUD			*/

//#define MPU9250_AD 0x68
//#define BMP280_AD 0x76

/* USART PORTS define	*/
#define UBRR  UBRR0
#define UCSRB UCSR0B
#define UDR   UDR0
#define RXEN  RXEN0
#define TXEN  TXEN0
#define RXCIE RXCIE0
#define UDRIE UDRIE0

/* USART interrupt control define	*/
#define USART_RECV_ENABLE()  ( UCSRB |=  _BV(RXCIE) )
#define USART_RECV_DISABLE() ( UCSRB &= ~_BV(RXCIE) )

#define USART_SEND_ENABLE()  ( UCSRB |=  _BV(UDRIE) )
#define USART_SEND_DISABLE() ( UCSRB &= ~_BV(UDRIE) )

#define millis() ((ctime * 10))

//Prototype declaration-------------
ISR( TIMER1_COMPA_vect );
ISR( USART_RX_vect );
ISR( USART_UDRE_vect );
uint8_t usart_recv_test( void );
uint8_t usart_getchar( void );
uint8_t usart_send_test( void );
void usart_putchar( char data );
static int _usart_putchar( char c ,FILE *stream );
static void hardwareInit( void );
//----------------------------------

double pre = 0, temp = 0, big_accX = 0, big_accY = 0, big_accZ = 0;
double ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0, temp2 = 0, tx = 0, ty = 0, tz = 0; 
int bmp280_interrupt_return = 0;
extern uint64_t ctime = 0;

typedef struct FIFO_t {				/* FIFO buffer struct	*/
	uint8_t idx_w;
	uint8_t idx_r;
	uint8_t count;
	uint8_t buff[FIFO_SIZE];
} FIFO;

static volatile FIFO txfifo;		/* send FIFO buffer 	*/
static volatile FIFO rxfifo;		/* recv FIFO buffer 	*/

/* Timer1 10msec Interval timer			*/
ISR( TIMER1_COMPA_vect )
{
	disk_timerproc();	/* MMC Timer	*/
    ctime++;
    i2c_timer = millis();
}

/* UART RECV interrupt					*/
ISR( USART_RX_vect )
{
	uint8_t data = UDR;
	if ( rxfifo.count < FIFO_SIZE ) {
		rxfifo.count++;
		rxfifo.buff[ rxfifo.idx_w++ ] = data;
		if ( rxfifo.idx_w >= FIFO_SIZE ) rxfifo.idx_w = 0;
	}
}

/* UART SEND registr empty interrupt	*/
ISR( USART_UDRE_vect )
{
	UDR = txfifo.buff[ txfifo.idx_r++ ];
	if ( txfifo.idx_r >= FIFO_SIZE ) txfifo.idx_r = 0;
	if ( --txfifo.count == 0 ) USART_SEND_DISABLE();
}

ISR(TWI_vect){
    bmp280_interrupt_return = bmp280_interrupt(bmp280_interrupt_return);
//    printf("%d\r\n",bmp280_interrupt_return);
}
/* USART Get recv data count			*/
uint8_t usart_recv_test( void )
{
	return ( rxfifo.count );
}

/* USART Get recv data byte				*/
uint8_t usart_getchar( void )
{
	uint8_t data;
	while ( !usart_recv_test() );
	USART_RECV_DISABLE();
	data = rxfifo.buff[ rxfifo.idx_r++ ];
	if ( rxfifo.idx_r >= FIFO_SIZE ) rxfifo.idx_r = 0;
	rxfifo.count--;
	USART_RECV_ENABLE();
	return ( data );
}

/* USART Get send buffer free size		*/
uint8_t usart_send_test( void )
{
	return ( FIFO_SIZE - txfifo.count );	/* calc free size */
}

/* USART Put send byte data				*/
void usart_putchar( char data )
{
	while ( !usart_send_test() );
	USART_SEND_DISABLE();
	txfifo.count++;
	txfifo.buff[ txfifo.idx_w++ ] = data;
	if ( txfifo.idx_w >= FIFO_SIZE ) txfifo.idx_w = 0;
	USART_SEND_ENABLE();
}

/* stdout setup							*/
static int _usart_putchar( char c ,FILE *stream )
{
	usart_putchar( c );
	if ( c == '\n' ) {
		usart_putchar( '\r' );
	}
	return ( 0 );
}
static FILE my_stdout = FDEV_SETUP_STREAM( _usart_putchar ,NULL ,_FDEV_SETUP_WRITE );

/* AVR Initialization					*/
static void hardwareInit( void )
{
	/* PortB  +--------7: Pullup : non use  */
	/*        |+-------6: Pullup : non use  */
	/*        ||+------5: HIZ    : SD SCK   */
	/*        |||+-----4: HIZ    : SD MISO  */
	/*        ||||+----3: HIZ    : SD MOSI  */
	/*        |||||+---2: Out(H) : SD #CS   */
	/*        ||||||+--1: Pullup : non use  */
	/*        |||||||+-0: Pullup : non use	*/
	DDRB  = 0b00000100;
	PORTB = 0b11000111;

	/* PortC  +--------7: Pullup : non use 	*/
	/*        |+-------6: Pullup : non use 	*/
	/*        ||+------5: Pullup : non use 	*/
	/*        |||+-----4: Pullup : non use 	*/
	/*        ||||+----3: Pullup : non use 	*/
	/*        |||||+---2: Pullup : non use 	*/
	/*        ||||||+--1: Pullup : non use	*/
	/*        |||||||+-0: Out(L) : SD #POW  */
	DDRC  = 0b00000001;
	PORTC = 0b11111110;

	/* PortD  +--------7: Pullup : non use	*/
	/*        |+-------6: Pullup : non use	*/
	/*        ||+------5: Pullup : non use	*/
	/*        |||+-----4: LED :	*/
	/*        ||||+----3: Pullup : non use	*/
	/*        |||||+---2: LED : */
	/*        ||||||+--1: Out(L) : TXD      */
	/*        |||||||+-0: Pullup : RXD      */
	DDRD  = 0b00010110;
	PORTD = 0b11111111;

	/* Timer1 setup							*/
	OCR1A = (uint16_t)( ( F_CPU / 256L ) / 100L );
	TCNT1 = 0;
	TCCR1B = 0x04 | _BV(WGM12);		/* F_CPU / 256 : CTC	*/
	TIMSK1 |= _BV(OCIE1A);

	/* UART Initialization					*/
	memset( (void *)&txfifo ,0x00 ,FIFO_SIZE );	/* TX FIFO buffer clear		*/
	memset( (void *)&rxfifo ,0x00 ,FIFO_SIZE );	/* RX FIFO buffer clear		*/
	UBRR = (uint16_t)( F_CPU / ( 16UL * (uint32_t)BAUD ) ) - 1;	/* set BAUD */
	UCSRB = _BV(RXEN) | _BV(TXEN);			/* RX / TX ENABLE				*/
	USART_RECV_ENABLE();					/* USART recv interrupt enable	*/

	stdout = &my_stdout;					/* Stdout setup					*/

	SMCR = _BV(SE); 						/* Power seve enabel for IDOL 	*/

    i2c_init();
    if(bmp280_init(&pre, &temp)) printf("init_err\r\n");
    if(ADXL375_init(&big_accX, &big_accY, &big_accZ)) printf("init_err\r\n");
    if(mpu9250_init(&ax, &ay, &az, &temp, &gx, &gy, &gz, &tx, &ty, &tz)) printf("init_err\r\n");
	sei();		  							/* enable global interrupt 		*/
}

/* MAIN						*/
int main( void )
{
	FRESULT res;
	FATFS fs;
	DIR dir;
	FILINFO info;
	FIL fil;
	uint16_t cnt;
	int ret;
    char buf[128];
    char dataFileName[16];
    uint8_t times;

    hardwareInit();
	printf( "start\r\n");
    ADXL375_setOffset(-0.35, -0.005, -0.50);
	DDRC |= (1 << PC1)|(1 << PC0);
	PORTC |= (1 << PC1);
    PORTC &= ~(1 << PC0);
    
//    f_mount( 0 ,&fs );								/* re mount			*/
//
//	strcpy( buf ,"<-------------------------------------------------->\n" );
//	printf( "%s" ,buf );
//	res = f_opendir( &dir ,"" );
//	if ( res != FR_OK ) {
//		printf( "f_opendir() : error %u\n\n" ,res );
//         while(1){
//	        PORTC ^= (1 << PC1)|(1 << PC0);
//            _delay_ms(500);
//         }
//	}
//
//
//	sprintf(dataFileName, "data.csv");	
//    res = f_open( &fil , dataFileName, FA_WRITE|FA_OPEN_ALWAYS);
//	if ( res != FR_OK ) {
//	     printf( "f_open() : error %u\n\n" ,res );
//         while(1){
//	        PORTC ^= (1 << PC1)|(1 << PC0);
//            _delay_ms(500);
//         }
//	 }
//    res = f_lseek(&fil, f_size(&fil));
//    f_close( &fil );
//
    while(1){    
//	    sprintf(dataFileName, "data.csv");	
//        res = f_open( &fil , dataFileName, FA_WRITE|FA_OPEN_ALWAYS);
//	    if ( res != FR_OK ) {
//	         printf( "f_open() : error %u\n\n" ,res );
//             while(1){
//	             PORTC ^= (1 << PC1)|(1 << PC0);
//               _delay_ms(500);
//          }  
//	     }  
//        res = f_lseek(&fil, f_size(&fil));
        mpu9250_getAcc();
        ADXL375_getAcc();
        bmp280_getAll();
        sprintf(buf ,"%lf,%lf,%lf,%lf\r\n",pre,temp,ax,big_accX);
//        ret = f_puts(buf,&fil);
        printf("%s",buf);
//        f_close( &fil );
	    PORTC ^= (1 << PC1)|(1 << PC0);
        _delay_ms(500);
    }
//    f_mount( 0 ,&fs );								/* re mount			*/

//	strcpy( buf ,"<-------------------------------------------------->\n" );
//	printf( "%s" ,buf );
//	res = f_opendir( &dir ,"" );
//	if ( res != FR_OK ) {
//		printf( "f_opendir() : error %u\n\n" ,res );
//	}
//
//
//	sprintf(dataFileName, "data.csv");	
//    while(1){
//        res = f_open( &fil , dataFileName, FA_WRITE|FA_OPEN_ALWAYS);
//	    if ( res != FR_OK ) {
//	        printf( "f_open() : error %u\n\n" ,res );
//	    }
//    
//        mpu9250_getAcc();
//        ADXL375_getAcc();
//        bmp280_getAll();
//        sprintf(buf ,"%ld,%lf,%lf,%lf,%lf\r\n",ctime*10,pre,temp,ax,big_accX);
//
//        ret = f_puts(buf,&fil);
//        f_close( &fil );
//
//	    PORTC ^= (1 << PC1)|(1 << PC0);
//        _delay_ms(500);
//    }
}
