/*###########################################################################
#                                                                           #
#                   		タイマーカウンターマクロ			　　        #
#																			#
# 以下のマクロを定義してからインクルードするとMS_WAIT()マクロの動作が変わる	#
# #define TIMER_NUM 0	0 : _delay_ms() 使用								#
#						1,3,4,5 : Timer使用(初期化は不要)					#
#																			#
###########################################################################*/
#ifndef __TIMER_H__
#define __TIMER_H__

#include <avr/io.h>
#include <util/delay.h>

/* 環境に合わせて使用するタイマーマクロを有効／無効設定(0:無効 1:有効) */
#define _TIMER0 1
#define _TIMER1 1
#define _TIMER2 1
#define _TIMER3 0
#define _TIMER4 0
#define _TIMER5 0

/* デフォルトマクロ				*/
#ifdef TIMER_NUM
	#if ( TIMER_NUM == 1 )
		#define TIMER_SET_TIMEOUT(tim) TIMER1_SET_TIMEOUT(tim)
		#define TIMER_IS_TIMEOUT() TIMER1_IS_TIMEOUT()
		#define TIMER_STOP() TIMER1_STOP()
		#define TIMER_WAIT_MS(tim) TIMER1_WAIT_MS(tim)
		#define TIMER_WAIT_SEC(tim) TIMER1_WAIT_SEC(tim)
	#elif ( TIMER_NUM == 3 )
		#define TIMER_SET_TIMEOUT(tim) TIMER3_SET_TIMEOUT(tim)
		#define TIMER_IS_TIMEOUT() TIMER3_IS_TIMEOUT()
		#define TIMER_STOP() TIMER3_STOP()
		#define TIMER_WAIT_MS(tim) TIMER3_WAIT_MS(tim)
		#define TIMER_WAIT_SEC(tim) TIMER3_WAIT_SEC(tim)
	#elif ( TIMER_NUM == 4 )
		#define TIMER_SET_TIMEOUT(tim) TIMER4_SET_TIMEOUT(tim)
		#define TIMER_IS_TIMEOUT() TIMER4_IS_TIMEOUT()
		#define TIMER_STOP() TIMER4_STOP()
		#define TIMER_WAIT_MS(tim) TIMER4_WAIT_MS(tim)
		#define TIMER_WAIT_SEC(tim) TIMER4_WAIT_SEC(tim)
	#elif ( TIMER_NUM == 5 )
		#define TIMER_SET_TIMEOUT(tim) TIMER_5SET_TIMEOUT(tim)
		#define TIMER_IS_TIMEOUT() TIMER5_IS_TIMEOUT()
		#define TIMER_STOP() TIMER5_STOP()
		#define TIMER_WAIT_MS(tim) TIMER5_WAIT_MS(tim)
		#define TIMER_WAIT_SEC(tim) TIMER5_WAIT_SEC(tim)
	#endif
#endif

#ifndef WAIT_MS
	#define WAIT_MS(t) SOFT_WAIT_MS(t)
#endif

/* ミリ秒をカウントに変換( F_CPU / 1024 で計算 ) */
#define TIMER_MS_TO_COUNT(tim) ( (uint16_t)( ( ( F_CPU / 1024L ) * (uint32_t)tim ) / 1000L ) )

/* delay.hのマクロを使用したウエイトマクロ */
#define SOFT_WAIT_MS(tim)								\
{														\
	uint16_t cnt = (uint16_t)(tim);						\
	while ( cnt-- ) {									\
		_delay_ms( 1 );									\
	}													\
}

#define SOFT_WAIT_SEC(tim)								\
{														\
	uint16_t cnt = (uint16_t)(tim);						\
	while ( cnt-- ) {									\
		SOFT_WAIT_MS( 1000 );							\
	}													\
}

/*****************************************************************************
*							タイマー０(8bir)マクロ
*****************************************************************************/
#if _TIMER0

/* msec単位のウエイト */
#define TIMER0_WAIT_MS( tim )				\
{											\
	uint16_t t = ( tim );					\
	OCR0A = TIMER_MS_TO_COUNT( 1 );			\
	TCCR0B = 0x05;							\
	while ( t-- ) {							\
		TCNT0 = 0;							\
		TIFR0 = _BV(OCF0A);					\
		while ( !( TIFR0 & _BV(OCF0A) ) );	\
	}										\
	OCR0A = 0;								\
}

/* sec単位のウエイト */
#define TIMER0_WAIT_SEC( tim )				\
{											\
	uint16_t t = ( tim );					\
	while ( t-- ) {							\
		TIMER0_WAIT_MS( 1000 );				\
	}										\
}

#endif

/*****************************************************************************
*							タイマー１(16bir)マクロ
*****************************************************************************/
#if _TIMER1

/* msec単位のタイムアウト値セット(最大4191msec : 16MHz駆動時) */
#define TIMER1_SET_TIMEOUT( tim )			\
{											\
	TCNT1 = 0;								\
	OCR1A = TIMER_MS_TO_COUNT( tim );		\
	TCCR1B = 0x05;							\
	TIFR1 = _BV(OCF1A);						\
}

/* TIMER1_SET_TIMEOUT()マクロで設定した時間が過ぎたかをチェック( 0:まだ !0:timeout ) */
#define TIMER1_IS_TIMEOUT()	( TIFR1 & _BV(OCF1A) )

/* タイマー停止					*/
#define TIMER1_STOP() { TCCR1B = 0; }

/* msec単位のウエイト(最大4191msec : 16MHz駆動時) */
#define TIMER1_WAIT_MS( tim )				\
{											\
	TIMER1_SET_TIMEOUT( tim );				\
	while ( !TIMER1_IS_TIMEOUT() );			\
	TCCR1B = 0;								\
}

/* sec単位のウエイト(最大255sec) */
#define TIMER1_WAIT_SEC( tim )				\
{											\
	uint8_t t = ( tim );					\
	while ( t-- ) {							\
		TIMER1_WAIT_MS( 1000 );				\
	}										\
}

#endif

/*****************************************************************************
*							タイマー２(8bir)マクロ
*****************************************************************************/

#if _TIMER2

/* msec単位のウエイト */
#define TIMER2_WAIT_MS( tim )				\
{											\
	uint16_t t = ( tim );					\
	OCR2A = TIMER_MS_TO_COUNT( 1 );			\
	TCCR2B = 0x07;							\
	while ( t-- ) {							\
		TCNT2 = 0;							\
		TIFR2 = _BV(OCF2A);					\
		while ( !( TIFR2 & _BV(OCF2A) ) );	\
	}										\
	TCCR2B = 0;								\
}

/* sec単位のウエイト */
#define TIMER2_WAIT_SEC( tim )				\
{											\
	uint16_t t = ( tim );					\
	while ( t-- ) {							\
		TIMER2_WAIT_MS( 1000 );				\
	}										\
}

#endif

/*****************************************************************************
*							タイマー３(16bit)マクロ
*****************************************************************************/

#if _TIMER3

/* msec単位のタイムアウト値セット(最大4191msec : 16MHz駆動時) */
#define TIMER3_SET_TIMEOUT( tim )			\
{											\
	TCNT3 = 0;								\
	OCR3A = TIMER_MS_TO_COUNT( tim );		\
	TCCR3B = 0x05;							\
	TIFR3 = _BV(OCF3A);						\
}

/* TIMER3_SET_TIMEOUT()マクロで設定した時間が過ぎたかをチェック( 0:まだ !0:timeout ) */
#define TIMER3_IS_TIMEOUT()	( TIFR3 & _BV(OCF3A) )

/* タイマー停止					*/
#define TIMER3_STOP() { TCCR3B = 0; }

/* msec単位のウエイト(最大4191msec : 16MHz駆動時) */
#define TIMER3_WAIT_MS( tim )				\
{											\
	TIMER3_SET_TIMEOUT( tim );				\
	while ( !TIMER3_IS_TIMEOUT() );			\
	TCCR3B = 0;								\
}

/* sec単位のウエイト(最大255sec) */
#define TIMER3_WAIT_SEC( tim )				\
{											\
	uint8_t t = ( tim );					\
	while ( t-- ) {							\
		TIMER3_WAIT_MS( 1000 );				\
	}										\
}

#endif

/*****************************************************************************
*							タイマー４(16bit)マクロ
*****************************************************************************/
#if _TIMER4

/* msec単位のタイムアウト値セット(最大4191msec : 16MHz駆動時) */
#define TIMER4_SET_TIMEOUT( tim )			\
{											\
	TCNT4 = 0;								\
	OCR4A = TIMER_MS_TO_COUNT( tim );		\
	TCCR4B = 0x05;							\
	TIFR4 = _BV(OCF4A);						\
}

/* TIMER4_SET_TIMEOUT()マクロで設定した時間が過ぎたかをチェック( 0:まだ !0:timeout ) */
#define TIMER4_IS_TIMEOUT()	( TIFR4 & _BV(OCF4A) )

/* タイマー停止					*/
#define TIMER4_STOP() { TCCR4B = 0; }

/* msec単位のウエイト(最大4191msec : 16MHz駆動時) */
#define TIMER4_WAIT_MS( tim )				\
{											\
	TIMER4_SET_TIMEOUT( tim );				\
	while ( !TIMER4_IS_TIMEOUT() );			\
	TCCR4B = 0;								\
}

/* sec単位のウエイト(最大255sec) */
#define TIMER4_WAIT_SEC( tim )				\
{											\
	uint8_t t = ( tim );					\
	while ( t-- ) {							\
		TIMER4_WAIT_MS( 1000 );				\
	}										\
}

#endif

/*****************************************************************************
*							タイマー５(16bit)マクロ
*****************************************************************************/
#if _TIMER5

/* msec単位のタイムアウト値セット(最大4191msec : 16MHz駆動時) */
#define TIMER5_SET_TIMEOUT( tim )			\
{											\
	TCNT5 = 0;								\
	OCR5A = TIMER_MS_TO_COUNT( tim );		\
	TCCR5B = 0x05;							\
	TIFR5 = _BV(OCF5A);						\
}

/* TIMER5_SET_TIMEOUT()マクロで設定した時間が過ぎたかをチェック( 0:まだ !0:timeout ) */
#define TIMER5_IS_TIMEOUT()	( TIFR5 & _BV(OCF5A) )

/* タイマー停止					*/
#define TIMER5_STOP() { TCCR5B = 0; }

/* msec単位のウエイト(最大4191msec : 16MHz駆動時) */
#define TIMER5_WAIT_MS( tim )				\
{											\
	TIMER5_SET_TIMEOUT( tim );				\
	while ( !TIMER5_IS_TIMEOUT() );			\
	TCCR5B = 0;								\
}

/* sec単位のウエイト(最大255sec) */
#define TIMER5_WAIT_SEC( tim )				\
{											\
	uint8_t t = ( tim );					\
	while ( t-- ) {							\
		TIMER5_WAIT_MS( 1000 );				\
	}										\
}

#endif

#endif
