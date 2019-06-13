/*###########################################################################
# FatFs用 R0.06～R0.08対応                                                  #
#							SDカード制御モジュール							#
#																			#
#                                 http://elm-chan.org/fsw/ff/00index_j.html #
#                                                            感謝!!  ChaN氏 #
###########################################################################*/
#define __MMC_C__

#include "mmc.h"
#include "../MYUTIL/spi_func.h"
#include "../MYUTIL/timer.h"
//#include "../rtc4543sa1.h"

/* BOOL定数定義						*/
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* SDカードコマンド定義				*/
#define CMD0	( 0x40 +  0 )		/* GO_IDLE_STATE 						*/
#define CMD1	( 0x40 +  1 )		/* SEND_OP_COND (MMC) 					*/
#define	ACMD41	( 0xc0 + 41 )		/* SEND_OP_COND (SDC) 					*/
#define CMD8	( 0x40 +  8 )		/* SEND_IF_COND 						*/
#define CMD9	( 0x40 +  9 )		/* SEND_CSD 							*/
#define CMD10	( 0x40 + 10 )		/* SEND_CID 							*/
#define CMD12	( 0x40 + 12 )		/* STOP_TRANSMISSION 					*/
#define ACMD13	( 0xc0 + 13 )		/* SD_STATUS (SDC) 						*/
#define CMD16	( 0x40 + 16 )		/* SET_BLOCKLEN 						*/
#define CMD17	( 0x40 + 17 )		/* READ_SINGLE_BLOCK 					*/
#define CMD18	( 0x40 + 18 )		/* READ_MULTIPLE_BLOCK 					*/
#define CMD23	( 0x40 + 23 )		/* SET_BLOCK_COUNT (MMC) 				*/
#define	ACMD23	( 0xc0 + 23 )		/* SET_WR_BLK_ERASE_COUNT (SDC) 		*/
#define CMD24	( 0x40 + 24 )		/* WRITE_BLOCK 							*/
#define CMD25	( 0x40 + 25 )		/* WRITE_MULTIPLE_BLOCK 				*/
#define CMD55	( 0x40 + 55 )		/* APP_CMD 								*/
#define CMD58	( 0x40 + 58 )		/* READ_OCR 							*/

/* タイムアウト検出タイマー制御マクロ */
#if ( SD_TIMER_TYPE == 0 )

	static volatile BYTE Timer1 = 0;	/* 100Hz decrement timer1			*/
	static volatile BYTE Timer2 = 0;	/* 100Hz decrement timer2			*/

	#define SET_TIME_OUT1(tim)	{ Timer1 = ( tim / 10 ); }
	#define IS_TIME_OUT1()		( Timer1 == 0 )
	#define STOP_TIMER1()

	#define SET_TIME_OUT2(tim)	{ Timer2 = ( tim / 10 ); }
	#define IS_TIME_OUT2()		( Timer2 == 0 )
	#define STOP_TIMER2()

	#define TIMER1_WAIT(tim)	{ Timer1 = ( tim / 10 ); while ( Timer1 ); }
	#define TIMER2_WAIT(tim)	{ Timer2 = ( tim / 10 ); while ( Timer2 ); }

#elif ( SD_TIMER_TYPE == 1 )
	#define SET_TIME_OUT1(tim)	TIMER1_SET_TIMEOUT(tim)
	#define IS_TIME_OUT1()		TIMER1_IS_TIMEOUT()
	#define STOP_TIMER1()		TIMER1_STOP()

	#define SET_TIME_OUT2(tim)	TIMER3_SET_TIMEOUT(tim)
	#define IS_TIME_OUT2()		TIMER3_IS_TIMEOUT()
	#define STOP_TIMER2()		TIMER3_STOP()

	#define TIMER1_WAIT(tim)	TIMER1_WAIT_MS(tim)
	#define TIMER2_WAIT(tim)	TIMER2_WAIT_MS(tim)

#elif ( SD_TIMER_TYPE == 2 )
	#define SET_TIME_OUT1(tim)	TIMER4_SET_TIMEOUT(tim)
	#define IS_TIME_OUT1()		TIMER4_IS_TIMEOUT()
	#define STOP_TIMER1()		TIMER4_STOP()

	#define SET_TIME_OUT2(tim)	TIMER5_SET_TIMEOUT(tim)
	#define IS_TIME_OUT2()		TIMER5_IS_TIMEOUT()
	#define STOP_TIMER2()		TIMER5_STOP()

	#define TIMER1_WAIT(tim)	TIMER4_WAIT_MS(tim)
	#define TIMER2_WAIT(tim)	TIMER5_WAIT_MS(tim)

#else
	#error Do not forget to set SD_TIMER_TYPE properly!
#endif

/* ローカル変数宣言						*/
static DSTATUS Stat = STA_NOINIT;	/* 現在のディスクステータス							*/
static BYTE CardType = 0;			/* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing	*/

/* ローカル関数プロトタイプ宣言			*/
static BYTE wait_ready( void );
static void power_on( void );
static void power_off( void );
static void release_spi( void );

#if ( _USE_IOCTL != 0 )
static int chk_power( void );
#endif

static uint8_t rcvr_datablock( BYTE *buff ,UINT btr );
static BYTE send_cmd( BYTE cmd ,DWORD arg );

#if ( _READONLY == 0 )
static uint8_t xmit_datablock( const BYTE *buff ,BYTE token );
#endif

/*****************************************************************************
*							ＳＤカードレディ待ち
* 引数：void
* 戻値：BYTE レスポンスデータ
*****************************************************************************/
static BYTE wait_ready( void )
{
	BYTE res;

	SET_TIME_OUT2( 500 );
	spiTransfer( 0xff );
	do {
		res = spiTransfer( 0xff );
	} while ( ( res != 0xff ) && !IS_TIME_OUT2() );
	STOP_TIMER2();

	return ( res );
}

/*****************************************************************************
*							SDカード電源オン
* 引数：void
* 戻値：void
*****************************************************************************/
static void power_on( void )
{
	/* SDカード電源ON				*/
#if USE_POWER
	SD_ENABLE_POWER();
#endif

	/* SPIポート初期化( CLK = LOW )	*/
	spiInit();

	/* 30msec待ち					*/
	TIMER1_WAIT( 30 );
}

/*****************************************************************************
*							SDカード電源オフ
* 引数：void
* 戻値：void
*****************************************************************************/
static void power_off( void )
{
	SD_ENABLE_CS();
	wait_ready();
	release_spi();

#if USE_POWER
	SD_DISABLE_POWER();
#endif

	Stat |= STA_NOINIT;		/* Set STA_NOINIT */
}

/*****************************************************************************
*						SPIバスからSDカードをリリース
* 引数：void
* 戻値：void
*****************************************************************************/
static void release_spi( void )
{
	SD_DISABLE_CS();
	spiTransfer( 0xff );
}

#if ( _USE_IOCTL != 0 )
/*****************************************************************************
*							SDカード電源状態取得
* 引数：void
* 戻値：int    0:OFF 1:ON
*****************************************************************************/
static int chk_power( void )
{
	#if USE_POWER
		return ( SD_GET_POWER()?0:1 );
	#else
		return ( 1 );
	#endif
}
#endif

/***************************************************************************
*							データパケット受信
* 引数：BYTE *buff 受信バッファ
*		UINT btr   受信バイト数(4の倍数)
* 戻値：uint8_t       TRUE/OK  FALSE/NG
***************************************************************************/
static uint8_t rcvr_datablock( BYTE *buff ,UINT btr )
{
	BYTE token;

	/* データパケットの開始を待つ(タイムアウト100msec)	*/
	SET_TIME_OUT1( 100 );
	do {
		token = spiTransfer( 0xff );
	} while ( ( token == 0xff ) && !IS_TIME_OUT1() );

	if ( token != 0xfe ) {
		return ( FALSE );
	}

	/* データパケット受信								*/
	do {
		*buff++ = spiTransfer( 0xff );
		*buff++ = spiTransfer( 0xff );
		*buff++ = spiTransfer( 0xff );
		*buff++ = spiTransfer( 0xff );
	} while ( btr -= 4 );

	/* CRCデータ読み捨て								*/
	spiTransfer( 0xff );
	spiTransfer( 0xff );

	return ( TRUE );
}

#if ( _READONLY == 0 )
/***************************************************************************
*						データパケット送信(512bytes)
* 引数：BYTE *buff 送信バッファ
*		BYTE token Data/Stop token
* 戻値：uint8_t       TRUE/OK  FALSE/NG
***************************************************************************/
static uint8_t xmit_datablock( const BYTE *buff ,BYTE token )
{
	BYTE resp;
	BYTE wc;

	if ( wait_ready() != 0xff ) {
		return ( FALSE );
	}

	spiTransfer( token );			/* Xmit data token 						*/
	if ( token != 0xfd ) {			/* Is data token						*/
		wc = 0;
		do {						/* Xmit the 512 byte data block to MMC	*/
			spiTransfer( *buff++ );
			spiTransfer( *buff++ );
		} while ( --wc );

		spiTransfer( 0xff );		/* CRC (Dummy) 							*/
		spiTransfer( 0xff );

		resp = spiTransfer( 0xff );	/* Reveive data response 				*/
		if ( ( resp & 0x1f ) != 0x05 ) {	/* If not accepted, return with error */
			return ( FALSE );
		}
	}

	return ( TRUE );
}
#endif		/* _READONLY == 0	*/

/***************************************************************************
*						コマンドパケット送信
* 引数：BYTE  cmd コマンド
*		DWORD arg パラメータ
* 戻値：BYTE      レスポンス
***************************************************************************/
static BYTE send_cmd( BYTE cmd ,DWORD arg )
{
	BYTE n;
	BYTE res;

	/* ACMDの場合、CMD55を最初に発行		*/
	if ( cmd & 0x80 ) {
		cmd &= 0x7F;
		res = send_cmd( CMD55 ,0 );
		if ( res > 1 ) {
			return ( res );
		}
	}

	/* Select the card and wait for ready	*/
	SD_DISABLE_CS();
	SD_ENABLE_CS();
	if ( wait_ready() != 0xff ) {
		return ( 0xff );
	}

	/* コマンドパケット送信					*/
	spiTransfer( cmd );
	spiTransfer( (BYTE)( arg >> 24 ) );			/* Argument[31..24]		*/
	spiTransfer( (BYTE)( arg >> 16 ) );			/* Argument[23..16]		*/
	spiTransfer( (BYTE)( arg >>  8 ) );			/* Argument[15..8]		*/
	spiTransfer( (BYTE)arg           );			/* Argument[7..0]		*/

	/* CRC送信								*/
	n = 0x01;									/* Dummy CRC + Stop		*/
	if ( cmd == CMD0 ) {						/* Valid CRC for CMD0	*/
		n = 0x95;
	}

	if ( cmd == CMD8 ) {						/* Valid CRC for CMD8	*/
		n = 0x87;
	}
	spiTransfer(n);

	/* Receive command response				*/
	if ( cmd == CMD12 ) {		/* Skip a stuff byte when stop reading	*/
		spiTransfer( 0xff );
	}

	/* Wait for a valid response in timeout of 10 attempts 				*/
	n = 10;
	do {
		res = spiTransfer( 0xff );
	} while ( ( res & 0x80 ) && --n );

	/* Return with the response value 		*/
	return ( res );
}

/*****************************************************************************
*							ドライブ初期化
* 引数：BYTE drv ドライブ番号(0固定)
* 戻値：DSTATUS  ディスクステータス
*****************************************************************************/
DSTATUS disk_initialize( BYTE drv )
{
	BYTE n;
	BYTE cmd;
	BYTE ty;
	BYTE ocr[4];

	if ( disk_status( 0 ) & STA_NODISK ) {
		return ( Stat );				/* No card in the socket			*/
	}

	power_on();							/* Force socket power on 			*/
	for ( n = 10 ;n ;n-- ) {			/* 80 dummy clocks 					*/
		spiTransfer( 0xff );
	}

	ty = 0;
	if ( send_cmd( CMD0 ,0 ) == 1 ) {	/* Enter Idle state 				*/
		SET_TIME_OUT1( 1000 );			/* Initialization timeout of 1sec	*/
		if ( send_cmd( CMD8 ,0x01aa ) == 1 ) {	/* SDHC 	*/
			for ( n = 0 ;n < 4 ;n++ ) {	/* Get trailing return value of R7 resp */
				ocr[n] = spiTransfer( 0xff );
			}
			if ( ( ocr[2] == 0x01 ) && ( ocr[3] == 0xaa ) ) {	/* The card can work at vdd range of 2.7-3.6V 				*/
				while ( !IS_TIME_OUT1() && send_cmd( ACMD41 ,1UL << 30 ) );	/* Wait for leaving idle state (ACMD41 with HCS bit)	*/
				if ( !IS_TIME_OUT1() && ( send_cmd( CMD58 ,0 ) == 0 ) ) {	/* Check CCS bit in the OCR								*/
					for ( n = 0 ;n < 4 ;n++ ) {
						ocr[n] = spiTransfer( 0xff );
					}
					ty = ( ocr[0] & 0x40 ) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
		} else {							/* SDSC or MMC	*/
			if ( send_cmd( ACMD41 ,0 ) <= 1 ) {
				ty = CT_SD1;
				cmd = ACMD41;				/* SDSC 		*/
			} else {
				ty = CT_MMC;
				cmd = CMD1;					/* MMC 			*/
			}

			while ( !IS_TIME_OUT1() && send_cmd( cmd ,0 ) );			/* Wait for leaving idle state 	*/
			if ( IS_TIME_OUT1() || ( send_cmd( CMD16 ,512 ) != 0 ) ) {	/* Set R/W block length to 512 	*/
				ty = 0;
			}
		}
		STOP_TIMER1();
	}

	CardType = ty;
	release_spi();

	if ( ty ) {								/* Initialization succeded			*/
		Stat &= ~STA_NOINIT;				/* Clear STA_NOINIT 				*/
		SPI_HI_SPEED();
	} else {								/* Initialization failed			*/
		power_off();
	}

	return ( Stat );
}

#if _USE_IOCTL != 0
/*****************************************************************************
*								ドライブ制御
* 引数：BYTE drv   ドライブ番号(0固定)
*		BYTE ctrl   CTRL_POWER			電源コントロール
*							  			buff[0] = 0 : power off
*							  			buff[0] = 1 : power on
*							  			buff[0] = 2 : 電源状態取得( buff[1] = OFF(0) / ON(1)
*					CTRL_SYNC 			同期(バッファクリア)
*					GET_SECTOR_COUNT	セクタ数取得( buff = DWORD )
*					GET_SECTOR_SIZE		セクタサイズ取得( buff = WORD )
*					GET_BLOCK_SIZE		ブロックサイズ取得( buff = DWORD )
*					MMC_GET_TYPE		カード種取得( buff = BYTE )
*					MMC_GET_CSD			CSD取得( buff = 16bytes )
*					MMC_GET_CID			CID取得( buff = 16bytes )
*					MMC_GET_OCR			OCR取得( buff = 4bytes )
*					MMC_GET_SDSTAT		SDステータス取得( buff = 64bytes )
*		void *buff  パラメータバッファ
* 戻値：DRESULT 	RES_OK   	0: Successful
*					RES_ERROR	1: R/W Error
*					RES_WRPRT	2: Write Protected
*					RES_NOTRDY	3: Not Ready
*					RES_PARERR	4: Invalid Parameter
*****************************************************************************/
DRESULT disk_ioctl( BYTE drv ,BYTE ctrl ,void *buff )
{
	BYTE n;
	BYTE csd[16];
	BYTE *ptr = buff;
	WORD csize;
	DRESULT res = RES_ERROR;

	if ( drv ) {
		return ( STA_NOINIT );			/* Supports only single drive			*/
	}

	if ( ctrl == CTRL_POWER ) {
		switch ( *ptr ) {
			case 0:						/* Sub control code == 0 (POWER_OFF)	*/
				if ( chk_power() ) {
 					power_off();		/* Power off */
				}
				res = RES_OK;
				break;
	
			case 1:						/* Sub control code == 1 (POWER_ON) 	*/
				power_on();				/* Power on	*/
				res = RES_OK;
				break;

			case 2:						/* Sub control code == 2 (POWER_GET)	*/
				*( ptr + 1 ) = (BYTE)chk_power();
				res = RES_OK;
				break;

			default:
				res = RES_PARERR;
				break;
		}

	} else {
		if ( disk_status( 0 ) & ( STA_NODISK | STA_NOINIT ) ) {
			return ( RES_NOTRDY );
		}

		switch ( ctrl ) {
			case CTRL_SYNC:				/* Make sure that no pending write process */
				SD_ENABLE_CS();
				if ( wait_ready() == 0xff ) {
					res = RES_OK;
				}
				break;

			case GET_SECTOR_COUNT:		/* Get number of sectors on the disk (DWORD) */
				if ( ( send_cmd( CMD9 ,0 ) == 0 ) && rcvr_datablock( csd ,16 ) ) {
					if ( ( csd[0] >> 6 ) == 1 ) {					/* SDC ver 2.00	 */
						csize = csd[9] + ( (WORD)csd[8] << 8 ) + 1;
						*(DWORD*)buff = (DWORD)csize << 10;
					} else {					/* SDC ver 1.XX or MMC*/
						n = ( csd[5] & 15 ) + ( ( csd[10] & 128) >> 7 ) + ( (csd[9] & 3 ) << 1 ) + 2;
						csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
						*(DWORD*)buff = (DWORD)csize << ( n - 9 );
					}
					res = RES_OK;
				}
				break;

			case GET_SECTOR_SIZE:		/* Get R/W sector size (WORD) 				*/
				*(WORD *)buff = 512;
				res = RES_OK;
				break;

			case GET_BLOCK_SIZE:		/* Get erase block size in unit of sector (DWORD) 	*/
				if ( CardType & CT_SD2 ) {							/* SDC ver 2.00 		*/
					if ( send_cmd( ACMD13 ,0 ) == 0 ) {				/* Read SD status 		*/
						spiTransfer( 0xff );
						if ( rcvr_datablock( csd ,16 ) ) {			/* Read partial block 	*/
							for ( n = 64 - 16 ;n ;n-- ) {
								spiTransfer( 0xff );				/* Purge trailing data 	*/
							}
							*(DWORD *)buff = 16UL << ( csd[10] >> 4 );
							res = RES_OK;
						}
					}
				} else {											/* SDC ver 1.XX or MMC	*/
					if ( ( send_cmd( CMD9 ,0 ) == 0 ) && rcvr_datablock( csd ,16 ) ) {	/* Read CSD */
						if ( CardType & CT_SD1 ) {					/* SDC ver 1.XX 		*/
							*(DWORD *)buff = ( ( ( csd[10] & 63 ) << 1 ) + ( (WORD)( csd[11] & 128) >> 7 ) + 1 ) << ( ( csd[13] >> 6 ) - 1 );
						} else {					/* MMC */
							*(DWORD*)buff = ( (WORD)( ( csd[10] & 124 ) >> 2 ) + 1 ) * ( ( ( csd[11] & 3 ) << 3 ) + ( ( csd[11] & 224 ) >> 5 ) + 1 );
						}
						res = RES_OK;
					}
				}
				break;

		case MMC_GET_TYPE:				/* Get card type flags (1 byte)						*/
			*ptr = CardType;
			res = RES_OK;
			break;

		case MMC_GET_CSD:				/* Receive CSD as a data block (16 bytes)			*/
			if ( ( send_cmd( CMD9 ,0 ) == 0 ) && rcvr_datablock( ptr ,16 ) ) {
				res = RES_OK;
			}
			break;

		case MMC_GET_CID:				/* Receive CID as a data block (16 bytes)			*/
			if ( ( send_cmd( CMD10 ,0 ) == 0 ) && rcvr_datablock( ptr ,16 ) ) {
				res = RES_OK;
			}
			break;

		case MMC_GET_OCR:				/* Receive OCR as an R3 resp (4 bytes)				*/
			if ( send_cmd( CMD58 ,0) == 0 ) {
				for ( n = 4 ;n ;n-- ) {
					*ptr++ = spiTransfer( 0xff );
				}
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT:			/* Receive SD status as a data block (64 bytes)		*/
			if ( send_cmd( ACMD13 ,0 ) == 0 ) {
				spiTransfer( 0xff );
				if ( rcvr_datablock( ptr ,64 ) ) {
					res = RES_OK;
				}
			}
			break;

		default:
			res = RES_PARERR;
		}

		release_spi();
	}

	return res;
}
#endif /* _USE_IOCTL != 0 */

/*****************************************************************************
*								セクタ読み込み
* 引数：BYTE  drv     ドライブ番号(0固定)
*		BYTE  *buff   読み込みバッファ
*		DWORD sector  読み込み開始セクタ番号
*		BYTE  count   セクタ数
* 戻値：DRESULT 	  RES_OK   	 0: Successful
*					  RES_ERROR	 1: R/W Error
*				  	  RES_WRPRT	 2: Write Protected
*					  RES_NOTRDY 3: Not Ready
*				      RES_PARERR 4: Invalid Parameter
*****************************************************************************/
DRESULT disk_read( BYTE drv ,BYTE *buff ,DWORD sector ,BYTE count )
{
	if ( disk_status( 0 ) & STA_NOINIT ) {
		return ( RES_NOTRDY );
	}

	if ( !( CardType & CT_BLOCK ) ) {	/* Convert to byte address if needed */
		sector *= 512L;
	}

	if ( count == 1 ) {					/* Single block read 				 */
		if ( ( send_cmd( CMD17 ,sector ) == 0 )	&& rcvr_datablock( buff ,512 ) ) {
			count = 0;
		}
	} else {							/* Multiple block read 				 */
		if ( send_cmd( CMD18 ,sector ) == 0 ) {		/* READ_MULTIPLE_BLOCK	 */
			do {
				if ( !rcvr_datablock( buff ,512 ) ) {
					break;
				}
				buff += 512;
			} while ( --count );
			send_cmd( CMD12 ,0 );		/* STOP_TRANSMISSION 				  */
		}
	}
	release_spi();

	return ( count ? RES_ERROR : RES_OK );
}

#if _READONLY == 0
/*****************************************************************************
*								セクタ書き込み
* 引数：BYTE        drv     ドライブ番号(0固定)
*		const BYTE  *buff   書き込みバッファ
*		DWORD       sector  書き込み開始セクタ番号
*		BYTE        count   セクタ数
* 戻値：DRESULT 	        RES_OK     0: Successful
*					        RES_ERROR  1: R/W Error
*				  	        RES_WRPRT  2: Write Protected
*					        RES_NOTRDY 3: Not Ready
*				            RES_PARERR 4: Invalid Parameter
*****************************************************************************/
DRESULT disk_write( BYTE drv ,const BYTE *buff ,DWORD sector ,BYTE count )
{
	if ( disk_status( 0 ) & STA_NOINIT ) {
		return ( RES_NOTRDY );
	}

	if ( !( CardType & CT_BLOCK ) ) {	/* Convert to byte address if needed */
		sector *= 512L;
	}

	if ( count == 1 ) {					/* Single block write 				 */
		if ( ( send_cmd( CMD24 ,sector ) == 0 )	&& xmit_datablock( buff, 0xfe ) ) {
			count = 0;
		}
	} else {							/* Multiple block write 			 */
		if ( CardType & CT_SDC ) {
			send_cmd( ACMD23 ,count );
		}
		if ( send_cmd( CMD25 ,sector ) == 0 ) {
			do {
				if ( !xmit_datablock( buff, 0xfc ) ) {
					break;
				}
				buff += 512;
			} while ( --count );
			if ( !xmit_datablock( 0 ,0xfd ) ) {		/* STOP_TRAN token		 */
				count = 1;
			}
		}
	}
	release_spi();

	return ( count ? RES_ERROR : RES_OK );
}
#endif /* _READONLY == 0 */

/*****************************************************************************
*							ドライブ状態取得
* 引数：BYTE drv ドライブ番号(0固定)
* 戻値：DSTATUS  ディスクステータス
*****************************************************************************/
DSTATUS disk_status( BYTE drv )
{
	if ( drv ) {
		return ( STA_NOINIT );			/* Supports only single drive 		*/
	}

#if USE_DT
	if ( SD_INS() ) {					/* SDカード挿入状態					*/
		Stat |= ( STA_NODISK | STA_NOINIT );
	} else {
		Stat &= ~STA_NODISK;
	}
#endif

	return ( Stat );
}

/*****************************************************************************
*								RTC取得
* 引数：void
* 戻値：DWORD 日時データ
* RTCが実装されていない場合は 0x00210000 : 1980/01/01 00:00:00 を固定で返す
*****************************************************************************/
DWORD get_fattime( void )
{
#if 0
	union {
		struct {
			uint8_t second  : 5;
			uint8_t minutes : 6;
			uint8_t hour    : 5;
			uint8_t day     : 5;
			uint8_t month   : 4;
			uint16_t year   : 7;
		} items;
		uint32_t val;
	} ft;

	RTC rtc;

	rtc_get( &rtc );

	ft.items.year = rtc.year - 1980;
	ft.items.month = rtc.month;
	ft.items.day = rtc.day;
	ft.items.hour = rtc.hour;
	ft.items.minutes = rtc.minutes;
	ft.items.second= rtc.second / 2;

	return ( ft.val );

#endif
	return ( 0x00210000 );
}

#if ( SD_TIMER_TYPE == 0 )
/*****************************************************************************
*						タイマープロセス
*						10msec毎に呼ぶこと
*****************************************************************************/
void disk_timerproc( void )
{
	if ( Timer1 ) {
		Timer1--;
	}

	if ( Timer2 ) {
		Timer2--;
	}

	disk_status( 0 );				/* DISKステータス更新	*/
}
#endif
