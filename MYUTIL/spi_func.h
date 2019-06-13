/*###########################################################################
#                                                                           #
#           	SPI���o�̓��W���[��(�}�X�^����mode0��p)            v0.20   #
#                                                                           #
#  �{�t�@�C�����C���N���[�h����O�Ɉȉ��̃}�N�����`���Ă�������           #
#  �{�t�@�C�����C���N���[�h����ʒu�ɋC������								#
#  �� #SS�|�[�g����̓��[�h�ɂ���Ɠ���(SPI-Port���}�X�^���삵�Ȃ��̂Œ���) #
#																			#
# #define SPI_TYPE 2			0:soft 1:SPIport 2:USART0 3:USART1			#
#                                                                           #
# #define SPI_MOSI_PIN PIND 	MOSI�Ɋ����Ă�|�[�g(PIN�|�[�g�Ŏw��)		#
# #define SPI_MOSI_BIT 1		MOSI�Ɋ����Ă�|�[�g�̃r�b�g�ԍ�			#
#                                                                           #
# #define SPI_MISO_PIN PIND		MISO�Ɋ����Ă�|�[�g(PIN�|�[�g�Ŏw��)		#
# #define SPI_MISO_BIT 0		MISO�Ɋ����Ă�|�[�g�̃r�b�g�ԍ�			#
#                                                                           #
# #define SPI_MISO_PIN PIND		SCK�Ɋ����Ă�|�[�g(PIN�|�[�g�Ŏw��)		#
# #define SPI_SCK_BIT 4			SCK�Ɋ��蓖�Ă�|�[�g�̃r�b�g�ԍ�			#
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
/* SPI�e��|�[�g��`						*/
/*											*/
#define SPI_MOSI_DDR  ( *(volatile uint8_t *)( &SPI_MOSI_PIN + 0x01 ) )
#define SPI_MOSI_PORT ( *(volatile uint8_t *)( &SPI_MOSI_PIN + 0x02 ) )

#define SPI_MISO_DDR  ( *(volatile uint8_t *)( &SPI_MISO_PIN + 0x01 ) )
#define SPI_MISO_PORT ( *(volatile uint8_t *)( &SPI_MISO_PIN + 0x02 ) )

#define SPI_SCK_DDR  ( *(volatile uint8_t *)( &SPI_SCK_PIN + 0x01 ) )
#define SPI_SCK_PORT ( *(volatile uint8_t *)( &SPI_SCK_PIN + 0x02 ) )

/*											*/
/* USART�|�[�g�ڑ����̃��W�X�^��`			*/
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
/* SPI�N���b�N���x�ݒ�}�N��				*/
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

/* SPI�|�[�g�������i�I�[�v���j�}�N��	*/
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
*								SPI �P�o�C�g����M
* �����Fuint8_t tx ���M�f�[�^
* �ߒl�Fuint8_t    ��M�f�[�^
*****************************************************************************/
static inline uint8_t spiTransfer( uint8_t tx )
{
	/* �\�t�g�E�G�ASPI���[�h(MODE0��p)	*/
	#if ( SPI_TYPE == 0 )
		uint8_t rx = 0x00;
		uint8_t bit = 8;

		while ( bit-- ) {

			rx <<= 1;						/* MISO �擾					*/
			if ( SPI_MISO_PIN & _BV(SPI_MISO_BIT) ) {
				rx |= 0x01;
			}

			if ( tx & 0x80 ) {				/* MOSI �o��					*/
				SPI_MOSI_PORT |=  _BV(SPI_MOSI_BIT);
			} else {
				SPI_MOSI_PORT &= ~_BV(SPI_MOSI_BIT);
			}
			tx <<= 1;

			SPI_SCK_PORT |= _BV(SPI_SCK_BIT);	/* SCK �o��					*/
			SPI_SCK_PORT &= ~_BV(SPI_SCK_BIT);
		}

		return ( rx );

	/* SPI�|�[�g�ڑ����[�h				*/
	#elif ( SPI_TYPE == 1 )
		SPDR = tx;							/* ���M�f�[�^���W�X�^�Z�b�g		*/
		while( !( SPSR & _BV(SPIF) ) );		/* �V�t�g�����҂�				*/
		return ( SPDR );					/* ��M�f�[�^��Ԃ�				*/

	/* USART�|�[�g�ڑ����[�h			*/
	#else
		while ( !( UCSRA & _BV(UDRE) ) );	/* ���M�f�[�^���W�X�^�󂫑҂�	*/
		UDR = tx;							/* ���M�f�[�^���W�X�^�Z�b�g		*/
		while ( !( UCSRA & _BV(RXC) ) );	/* �f�[�^��M�����҂�			*/
		return ( UDR );						/* ��M�f�[�^��Ԃ�				*/
	#endif
}

#endif
