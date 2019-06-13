/*###########################################################################
# FatFs用 R0.06～R0.08対応                                                  #
#							SDカード制御モジュール							#
#																			#
#                                 http://elm-chan.org/fsw/ff/00index_j.html #
#                                                            感謝!!  ChaN氏 #
###########################################################################*/
#ifndef __MMC_H__
#define __MMC_H__

#include <avr/io.h>
#include "../FatFs/ff.h"
#include "../FatFs/diskio.h"

#define USE_POWER	1				/* SDカード電源制御  0:未使用 1:使用	*/
#define USE_DT		0				/* SDカード挿入状態取得 0:未使用 1:使用	*/

/* タイマー選択						*/
#ifdef __MMC_C__
#define SD_TIMER_TYPE 0				/* 0 : ChaN様オリジナルタイムアウト検出	*/
									/*     disk_timerproc()を使用			*/
									/* 1 : timer 1 と 3 を使用				*/
									/* 2 : timer 4 と 5 を使用				*/

/* SPIポート定義					*/
#define SPI_TYPE 1					/* 0:soft 1:SPIport 2:USART0 3:USART1	*/

#define SPI_MOSI_PIN  PINB			/* MOSIに割当てるポート(PINで指定)		*/
#define SPI_MOSI_BIT  3				/* MOSIに割当てるポートのビット番号		*/

#define SPI_MISO_PIN  PINB			/* MISOに割当てるポート(PINで指定)		*/
#define SPI_MISO_BIT  4				/* MISOに割当てるポートのビット番号		*/

#define SPI_SCK_PIN   PINB			/* SCKに割当てるポート(PINで指定)		*/
#define SPI_SCK_BIT   5				/* SCKに割当てるポートのビット番号		*/

/* CS信号制御マクロ定義				*/
#define SD_ENABLE_CS()		( PORTB &= ~_BV(PB2) )	/* HI:OFF LOW:ON			*/
#define SD_DISABLE_CS()		( PORTB |=  _BV(PB2) )

/* SDカード電源制御マクロ定義		*/
#if USE_POWER
#define SD_ENABLE_POWER()	( PORTC &= ~_BV(PC0) )	/* HI:OFF LOW:ON			*/
#define SD_DISABLE_POWER()	( PORTC |=  _BV(PC0) )
#define SD_GET_POWER()		( PORTC &   _BV(PC0) )
#else 
#define SD_ENABLE_POWER()
#define SD_DISABLE_POWER()
#define SD_GET_POWER() 0
#endif

#endif	// #ifdef __MMC_C__

/* SDカード挿入状態取得マクロ定義	*/
#if USE_DT
#define SD_INS() ( PINB & _BV(PB7) )				/* !0:未挿入  0:挿入中		*/
#else
#define SD_INS() ( 0 )								/* !0:未挿入  0:挿入中		*/
#endif	// #if USE_DT

/* カードタイプ定義				*/
#define CT_MMC			0x01
#define CT_SD1			0x02
#define CT_SD2			0x04
#define CT_SDC			( CT_SD1 | CT_SD2 )
#define CT_BLOCK		0x08

/* 関数プロトタイプ宣言			*/
DSTATUS disk_initialize( BYTE drv );
DRESULT disk_ioctl( BYTE drv ,BYTE ctrl ,void *buff );
DRESULT disk_read( BYTE drv ,BYTE *buff ,DWORD sector ,BYTE count );
DRESULT disk_write( BYTE drv ,const BYTE *buff ,DWORD sector ,BYTE count );
DSTATUS disk_status( BYTE drv );
DWORD get_fattime( void );

#if ( SD_TIMER_TYPE == 0 )
void disk_timerproc( void );
#endif

#endif		// #ifndef __MMC_H__
