/*###########################################################################
#                                                                           #
#           	SPI入出力モジュール(マスタ動作mode0専用)            v0.20   #
#                                                                           #
#  本ファイルをインクルードする前に以下のマクロを定義しておくこと           #
#  本ファイルをインクルードする位置に気をつける								#
#  ※ #SSポートを入力モードにすると動作(SPI-Portがマスタ動作しないので注意) #
#																			#
# #define SPI_TYPE 2			0:soft 1:SPIport 2:USART0 3:USART1			#
#                                                                           #
# #define SPI_MOSI_PIN PIND 	MOSIに割当てるポート(PINポートで指定)		#
# #define SPI_MOSI_BIT 1		MOSIに割当てるポートのビット番号			#
#                                                                           #
# #define SPI_MISO_PIN PIND		MISOに割当てるポート(PINポートで指定)		#
# #define SPI_MISO_BIT 0		MISOに割当てるポートのビット番号			#
#                                                                           #
# #define SPI_MISO_PIN PIND		SCKに割当てるポート(PINポートで指定)		#
# #define SPI_SCK_BIT 4			SCKに割り当てるポートのビット番号			#
#                                                                           #
###########################################################################*/
#ifndef __SPI_FUNC_H__
#define __SPI_FUNC_H__

#include <avr/io.h>

#ifndef SPI_TYPE
#error Do not forget to set SPI_TYPE
#endif

#ifndef SPI_MOSI_PIN
#error Do not forget to set SPI_MOSI_PIN
#endif

#ifndef SPI_MOSI_BIT
#error Do not forget to set SPI_MOSI_BIT
#endif

#ifndef SPI_MISO_PIN
#error Do not forget to set SPI_MISO_PIN
#endif

#ifndef SPI_MISO_BIT
#error Do not forget to set SPI_MISO_BIT
#endif

#ifndef SPI_SCK_PIN
#error Do not forget to set SPI_SCK_PIN
#endif

#ifndef SPI_SCK_BIT
#error Do not forget to set SPI_SCK_BIT
#endif

/*											*/
/* SPI各種ポート定義						*/
/*											*/
#define SPI_MOSI_DDR  ( *(volatile uint8_t *)( &SPI_MOSI_PIN + 0x01 ) )
#define SPI_MOSI_PORT ( *(volatile uint8_t *)( &SPI_MOSI_PIN + 0x02 ) )

#define SPI_MISO_DDR  ( *(volatile uint8_t *)( &SPI_MISO_PIN + 0x01 ) )
#define SPI_MISO_PORT ( *(volatile uint8_t *)( &SPI_MISO_PIN + 0x02 ) )

#define SPI_SCK_DDR  ( *(volatile uint8_t *)( &SPI_SCK_PIN + 0x01 ) )
#define SPI_SCK_PORT ( *(volatile uint8_t *)( &SPI_SCK_PIN + 0x02 ) )

/*											*/
/* USARTポート接続時のレジスタ定義			*/
/*											*/
#if ( SPI_TYPE == 2 )
#define UBRR  UBRR0
#define UCSRA UCSR0A
#define UDRE  UDRE0
#define UDR   UDR0
#define TXC   TXC0
#define RXC   RXC0

#elif ( SPI_TYPE == 3 )
#define UBRR  UBRR1
#define UCSRA UCSR1A
#define UDRE  UDRE1
#define UDR   UDR1
#define TXC   TXC1
#define RXC   RXC1

#elif ( ( SPI_TYPE != 0 ) && ( SPI_TYPE != 1 ) )
#error Do not forget to set SPI_TYPE
#endif

/*											*/
/* SPIクロック速度設定マクロ				*/
/*											*/
#if ( SPI_TYPE == 0 )
#define SPI_LOW_SPEED()
#define SPI_MID_SPEED()
#define SPI_HI_SPEED()

#elif ( SPI_TYPE == 1 )
#define SPI_LOW_SPEED()					/* F_CPU / 128	*/					\
{																			\
	SPCR = 0x00;															\
	SPSR = 0x00;															\
	SPCR = _BV(SPR1) | _BV(SPR0) | _BV(MSTR) | _BV(SPE);					\
}
#define SPI_MID_SPEED()					/* F_CPU / 4	*/					\
{																			\
	SPCR = 0x00;															\
	SPSR = 0x00;															\
	SPCR = _BV(MSTR) | _BV(SPE);											\
}
#define SPI_HI_SPEED()					/* F_CPU / 2	*/					\
{																			\
	SPCR = 0x00;															\
	SPSR = _BV(SPI2X);														\
	SPCR = _BV(MSTR) | _BV(SPE);											\
}
#else

#define SPI_LOW_SPEED()					/* F_CPU / 128	*/					\
{																			\
	UBRR = 63;																\
}
#define SPI_MID_SPEED()					/* F_CPU / 4	*/					\
{																			\
	UBRR = 1;																\
}
#define SPI_HI_SPEED()					/* F_CPU / 2	*/					\
{																			\
	UBRR = 0;																\
}
#endif

/* SPIポート初期化（オープン）マクロ	*/
#if ( SPI_TYPE == 0 )
#define spiInit()															\
	SPI_MOSI_DDR |= _BV(SPI_MOSI_BIT);										\
	SPI_MOSI_PORT &= ~_BV(SPI_MOSI_BIT);									\
																			\
	SPI_MISO_DDR &= ~_BV(SPI_MISO_BIT);										\
	SPI_MISO_PORT |= _BV(SPI_MISO_BIT);										\
																			\
	SPI_SCK_DDR |= _BV(SPI_SCK_BIT);										\
	SPI_SCK_PORT &= ~_BV(SPI_SCK_BIT);										\
																			\
	SPI_LOW_SPEED();														\

#elif ( SPI_TYPE == 1 )
#define spiInit()															\
{																			\
	SPCR = 0x00;															\
																			\
	SPI_MOSI_DDR |= _BV(SPI_MOSI_BIT);										\
	SPI_MOSI_PORT &= ~_BV(SPI_MOSI_BIT);									\
																			\
	SPI_MISO_DDR &= ~_BV(SPI_MISO_BIT);										\
	SPI_MISO_PORT |= _BV(SPI_MISO_BIT);										\
																			\
	SPI_SCK_DDR |= _BV(SPI_SCK_BIT);										\
	SPI_SCK_PORT &= ~_BV(SPI_SCK_BIT);										\
																			\
	SPI_LOW_SPEED();														\
}

#elif ( SPI_TYPE == 2 )
#define spiInit()															\
{																			\
	UCSR0B = 0;																\
	UCSR0C = 0;																\
																			\
	SPI_MOSI_DDR |= _BV(SPI_MOSI_BIT);										\
	SPI_MOSI_PORT &= ~_BV(SPI_MOSI_BIT);									\
																			\
	SPI_MISO_DDR &= ~_BV(SPI_MISO_BIT);										\
	SPI_MISO_PORT |= _BV(SPI_MISO_BIT);										\
																			\
	SPI_SCK_DDR |= _BV(SPI_SCK_BIT);										\
	SPI_SCK_PORT &= ~_BV(SPI_SCK_BIT);										\
																			\
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);										\
	UCSR0C = 0xc0;															\
																			\
	SPI_LOW_SPEED();														\
}

#elif ( SPI_TYPE == 3 )
#define spiInit()															\
{																			\
	UCSR1B = 0;																\
	UCSR1C = 0;																\
																			\
	SPI_MOSI_DDR |= _BV(SPI_MOSI_BIT);										\
	SPI_MOSI_PORT &= ~_BV(SPI_MOSI_BIT);									\
																			\
	SPI_MISO_DDR &= ~_BV(SPI_MISO_BIT);										\
	SPI_MISO_PORT |= _BV(SPI_MISO_BIT);										\
																			\
	SPI_SCK_DDR |= _BV(SPI_SCK_BIT);										\
	SPI_SCK_PORT &= ~_BV(SPI_SCK_BIT);										\
																			\
	UCSR1B = _BV(RXEN1) | _BV(TXEN1);										\
	UCSR1C = 0xc0;															\
																			\
	SPI_LOW_SPEED();														\
}
#endif

/*****************************************************************************
*								SPI １バイト送受信
* 引数：uint8_t tx 送信データ
* 戻値：uint8_t    受信データ
*****************************************************************************/
static inline uint8_t spiTransfer( uint8_t tx )
{
	/* ソフトウエアSPIモード(MODE0専用)	*/
	#if ( SPI_TYPE == 0 )
		uint8_t rx = 0x00;
		uint8_t bit = 8;

		while ( bit-- ) {

			rx <<= 1;						/* MISO 取得					*/
			if ( SPI_MISO_PIN & _BV(SPI_MISO_BIT) ) {
				rx |= 0x01;
			}

			if ( tx & 0x80 ) {				/* MOSI 出力					*/
				SPI_MOSI_PORT |=  _BV(SPI_MOSI_BIT);
			} else {
				SPI_MOSI_PORT &= ~_BV(SPI_MOSI_BIT);
			}
			tx <<= 1;

			SPI_SCK_PORT |= _BV(SPI_SCK_BIT);	/* SCK 出力					*/
			SPI_SCK_PORT &= ~_BV(SPI_SCK_BIT);
		}

		return ( rx );

	/* SPIポート接続モード				*/
	#elif ( SPI_TYPE == 1 )
		SPDR = tx;							/* 送信データレジスタセット		*/
		while( !( SPSR & _BV(SPIF) ) );		/* シフト完了待ち				*/
		return ( SPDR );					/* 受信データを返す				*/

	/* USARTポート接続モード			*/
	#else
		while ( !( UCSRA & _BV(UDRE) ) );	/* 送信データレジスタ空き待ち	*/
		UDR = tx;							/* 送信データレジスタセット		*/
		while ( !( UCSRA & _BV(RXC) ) );	/* データ受信完了待ち			*/
		return ( UDR );						/* 受信データを返す				*/
	#endif
}

#endif
