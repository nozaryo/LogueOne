/*															*/
/* AtMega328p/@16MHz USART 9600BAUD							*/
/*															*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "senser/i2c.h"
#include "FatFs/ff.h"
#include "MMC/mmc.h"

#define FIFO_SIZE 16				/* FIFO buffer size		*/
#define BAUD 9600					/* USART BAUD			*/

#define MPU9250_AD 0x68
#define BMP280_AD 0x76

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
    i2c_writeSet(MPU9250_AD,0x37,0b00000010);
    i2c_writeSet(MPU9250_AD,0x1C,0b00010000);
    i2c_writeSet(MPU9250_AD,0x1B,0b00011000);
    i2c_writeSet(0x0C,0x0A,0b00000110);
    i2c_writeSet(BMP280_AD, 0xF5, 0x90);
	sei();		  							/* enable global interrupt 		*/
}

/* MAIN						*/
int main( void )
{
    hardwareInit();

    while(1){
    }
}
