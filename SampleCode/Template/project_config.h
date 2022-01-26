/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "NuMicro.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
extern volatile uint32_t BitFlag;

enum
{
	_eep_index_RTC_is_record								= 0x00 + 1*4,	
	_eep_index_RTC_Year_LSB									= 0x00 + 2*4,	
	_eep_index_RTC_Year_MSB									= 0x00 + 3*4,
	_eep_index_RTC_Month									= 0x00 + 4*4,
	_eep_index_RTC_Day										= 0x00 + 5*4,
	_eep_index_RTC_DayOfWeek								= 0x00 + 6*4,	
	_eep_index_RTC_Hour										= 0x00 + 7*4,	
	_eep_index_RTC_Minute									= 0x00 + 8*4,	
	_eep_index_RTC_Second									= 0x00 + 9*4,	
	_eep_index_RTC_TimeScale								= 0x00 + 10*4,		
	_eep_index_RTC_AmPm										= 0x00 + 11*4,

	_eep_index_default ,

};


typedef enum{
	flag_start = 0 ,

	flag_wake_up_by_timer ,

	flag_error ,		
	flag_DEFAULT	
}Flag_Index;

/*_____ D E F I N I T I O N S ______________________________________________*/
#define _debug_log_UART_					(1)


/*_____ M A C R O S ________________________________________________________*/

#define BitFlag_ON(flag)							(BitFlag|=flag)
#define BitFlag_OFF(flag)							(BitFlag&=~flag)
#define BitFlag_READ(flag)							((BitFlag&flag)?1:0)
#define ReadBit(bit)								(uint32_t)(1<<bit)

#define is_flag_set(idx)							(BitFlag_READ(ReadBit(idx)))
#define set_flag(idx,en)							( (en == 1) ? (BitFlag_ON(ReadBit(idx))) : (BitFlag_OFF(ReadBit(idx))))

//MACRO : Swap Integers Macro
#ifndef SWAP
#define SWAP(a, b)     							{(a) ^= (b); (b) ^= (a); (a) ^= (b);}
#endif

#ifndef MAX
#define MAX(a,b) 								(((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) 								(((a) < (b)) ? (a) : (b))
#endif

//16 --> 8 x 2
#ifndef HIBYTE
#define HIBYTE(v1)              					((uint8_t)((v1)>>8))                      		//v1 is uint16_t
#endif

#ifndef LOBYTE
#define LOBYTE(v1)              					((uint8_t)((v1)&0xFF))
#endif

//8 x 2 --> 16
#ifndef MAKEWORD
#define MAKEWORD(v1,v2)         				((((uint16_t)(v1))<<8)+(uint16_t)(v2))      //v1,v2 is uint8_t
#endif

//8 x 4 --> 32
#ifndef MAKELONG
#define MAKELONG(v1,v2,v3,v4)   					(uint32_t)((v1<<24)+(v2<<16)+(v3<<8)+v4)  //v1,v2,v3,v4 is uint8_t
#endif

#ifndef SIZEOF
#define SIZEOF(a) 								(sizeof(a) / sizeof(a[0]))
#endif

#ifndef ENDOF
#define ENDOF(a) 								((a) + SIZEOF(a))
#endif

#ifndef CLEAR
#define CLEAR(x) 								(memset(&(x), 0, sizeof (x)))
#endif

#ifndef ABS
#define ABS(X)  									((X) > 0 ? (X) : -(X)) 
#endif

/*_____ F U N C T I O N S __________________________________________________*/
void reset_buffer(void *dest, unsigned int val, unsigned int size);
void copy_buffer(void *dest, void *src, unsigned int size);
void dump_buffer(uint8_t *pucBuff, int nBytes);
void  dump_buffer_hex(uint8_t *pucBuff, int nBytes);




