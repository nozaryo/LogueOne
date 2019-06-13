/*###########################################################################
# FatFs�p R0.06�`R0.08�Ή�                                                  #
#							SD�J�[�h���䃂�W���[��							#
#																			#
#                                 http://elm-chan.org/fsw/ff/00index_j.html #
#                                                            ����!!  ChaN�� #
###########################################################################*/
#ifndef __MMC_H__
#define __MMC_H__

#include <avr/io.h>
#include "../FatFs/ff.h"
#include "../FatFs/diskio.h"

#define USE_POWER	1				/* SD�J�[�h�d������  0:���g�p 1:�g�p	*/
#define USE_DT		0				/* SD�J�[�h�}����Ԏ擾 0:���g�p 1:�g�p	*/

/* �^�C�}�[�I��						*/
#ifdef __MMC_C__
#define SD_TIMER_TYPE 0				/* 0 : ChaN�l�I���W�i���^�C���A�E�g���o	*/
									/*     disk_timerproc()���g�p			*/
									/* 1 : timer 1 �� 3 ���g�p				*/
									/* 2 : timer 4 �� 5 ���g�p				*/

/* SPI�|�[�g��`					*/
#define SPI_TYPE 1					/* 0:soft 1:SPIport 2:USART0 3:USART1	*/

#define SPI_MOSI_PIN  PINB			/* MOSI�Ɋ����Ă�|�[�g(PIN�Ŏw��)		*/
#define SPI_MOSI_BIT  3				/* MOSI�Ɋ����Ă�|�[�g�̃r�b�g�ԍ�		*/

#define SPI_MISO_PIN  PINB			/* MISO�Ɋ����Ă�|�[�g(PIN�Ŏw��)		*/
#define SPI_MISO_BIT  4				/* MISO�Ɋ����Ă�|�[�g�̃r�b�g�ԍ�		*/

#define SPI_SCK_PIN   PINB			/* SCK�Ɋ����Ă�|�[�g(PIN�Ŏw��)		*/
#define SPI_SCK_BIT   5				/* SCK�Ɋ����Ă�|�[�g�̃r�b�g�ԍ�		*/

/* CS�M������}�N����`				*/
#define SD_ENABLE_CS()		( PORTB &= ~_BV(PB2) )	/* HI:OFF LOW:ON			*/
#define SD_DISABLE_CS()		( PORTB |=  _BV(PB2) )

/* SD�J�[�h�d������}�N����`		*/
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

/* SD�J�[�h�}����Ԏ擾�}�N����`	*/
#if USE_DT
#define SD_INS() ( PINB & _BV(PB7) )				/* !0:���}��  0:�}����		*/
#else
#define SD_INS() ( 0 )								/* !0:���}��  0:�}����		*/
#endif	// #if USE_DT

/* �J�[�h�^�C�v��`				*/
#define CT_MMC			0x01
#define CT_SD1			0x02
#define CT_SD2			0x04
#define CT_SDC			( CT_SD1 | CT_SD2 )
#define CT_BLOCK		0x08

/* �֐��v���g�^�C�v�錾			*/
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
