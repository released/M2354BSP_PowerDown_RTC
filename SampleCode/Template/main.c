/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "project_config.h"
#include "DataFlashProg.h"


/*_____ D E C L A R A T I O N S ____________________________________________*/

#define STANDBY_GPIO_PORT			(PA)
#define STANDBY_GPIO_PIN_MASK		(BIT2)
#define STANDBY_GPIO_PIN			(2)
#define STANDBY_GPIO				(PA2)

#define SW_GPIO_PORT				(PA)
#define SW_GPIO_PIN_MASK			(BIT1)
#define SW_GPIO_PIN					(1)
#define SW_GPIO						(PA1)


/*_____ D E F I N I T I O N S ______________________________________________*/
volatile uint32_t BitFlag = 0;
volatile uint32_t counter_tick = 0;

uint8_t report_flag = 0;

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , ENABLE);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("%s finish \r\n" , __FUNCTION__);	
		set_flag(flag_error , DISABLE);
	}

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void  dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}


void PowerDownFunction(void)
{
    /* Check if all the debug messages are finished */
    UART_WAIT_TX_EMPTY(DEBUG_PORT);

    /* Enter to Power-down mode */
    CLK_PowerDown();
}

void WakeUpTimerFunction(uint32_t u32PDMode, uint32_t u32Interval)
{
	/*
		Wake-up Timer Time-out Interval Select Bits (Write Protect) 
		These bits control wake-up timer time-out interval when chip under Deep Power-down mode or Standby 
		Power-down mode. 
		000 = Time-out interval is 410 LIRC clocks (12.8ms). 
		001 = Time-out interval is 819 LIRC clocks (25.6ms). 
		010 = Time-out interval is 1638 LIRC clocks (51.2ms). 
		011 = Time-out interval is 3277 LIRC clocks (102.4ms). 
		100 = Time-out interval is 13107 LIRC clocks (409.6ms). 
		101 = Time-out interval is 26214 LIRC clocks (819.2ms). 
		110 = Time-out interval is 52429 LIRC clocks (1638.4ms). 
		111 = Time-out interval is 209715 LIRC clocks (6553.6ms). 
		Note: These bits are write protected. Refer to the SYS_REGLCTL register. 
	
*/

    /* Unlock protected registers before setting Power-down mode */
    SYS_UnlockReg();

    printf("Enter to DPD Power-down mode......\n");

    /* Select Power-down mode */
    CLK_SetPowerDownMode(u32PDMode);

    /* Set Wake-up Timer Time-out Interval */
    CLK_SET_WKTMR_INTERVAL(u32Interval);

    /* Enable Wake-up Timer */
    CLK_ENABLE_WKTMR();

    /* Enter to Power-down mode */
    PowerDownFunction();
}

void WakeUpRTCTickFunction(uint32_t u32PDMode)
{
    printf("Enter to DPD Power-down mode......\n");

    /* Enable RTC peripheral clock */
    CLK->APBCLK0 |= CLK_APBCLK0_RTCCKEN_Msk;

    /* RTC clock source select LXT */
    CLK_SetModuleClock(RTC_MODULE, RTC_LXTCTL_RTCCKSEL_LXT, (uint32_t)NULL);

    /* Open RTC and start counting */
    RTC->INIT = RTC_INIT_KEY;
    if(RTC->INIT != RTC_INIT_ACTIVE_Msk)
    {
        RTC->INIT = RTC_INIT_KEY;
        while(RTC->INIT != RTC_INIT_ACTIVE_Msk);
    }

    /* Clear tick status */
    RTC_CLEAR_TICK_INT_FLAG(RTC);

    /* Enable RTC Tick interrupt */
    RTC_EnableInt(RTC_INTEN_TICKIEN_Msk);

    /* Select Power-down mode */
    CLK_SetPowerDownMode(u32PDMode);

    /* Set RTC tick period as 1 second */
    RTC_SetTickPeriod(RTC_TICK_1_8_SEC);

    /* Enable RTC wake-up */
    CLK_ENABLE_RTCWK();

    /* Enter to Power-down mode */
    PowerDownFunction();
}


void disable_RTC_tick(void)
{
    RTC_DisableInt(RTC_INTEN_TICKIEN_Msk);
    NVIC_DisableIRQ(RTC_IRQn);
}

void RTC_report_log(void)
{
	S_RTC_TIME_DATA_T sReadRTC; 

	/*
		1. record data in flash
		2. if request by external and report by UART
				clear log

	*/

	RTC_GetDateAndTime(&sReadRTC);
	printf("# Get RTC current date/time: %d/%02d/%02d %02d:%02d:%02d.\n",
		   sReadRTC.u32Year, sReadRTC.u32Month, sReadRTC.u32Day, sReadRTC.u32Hour, sReadRTC.u32Minute, sReadRTC.u32Second);

	#if 1
	printf("record RTC data START\r\n");
	FLASH_Unlock();
	Emulate_EEPROM_Write(_eep_index_RTC_Year_MSB, HIBYTE(sReadRTC.u32Year));
	Emulate_EEPROM_Write(_eep_index_RTC_Year_LSB, LOBYTE(sReadRTC.u32Year));	

	Emulate_EEPROM_Write(_eep_index_RTC_Month, LOBYTE(sReadRTC.u32Month));
	Emulate_EEPROM_Write(_eep_index_RTC_Day, LOBYTE(sReadRTC.u32Day));
	Emulate_EEPROM_Write(_eep_index_RTC_DayOfWeek, LOBYTE(sReadRTC.u32DayOfWeek));
	Emulate_EEPROM_Write(_eep_index_RTC_Hour, LOBYTE(sReadRTC.u32Hour));
	Emulate_EEPROM_Write(_eep_index_RTC_Minute, LOBYTE(sReadRTC.u32Minute));
	Emulate_EEPROM_Write(_eep_index_RTC_Second, LOBYTE(sReadRTC.u32Second));
	Emulate_EEPROM_Write(_eep_index_RTC_TimeScale, LOBYTE(sReadRTC.u32TimeScale));
	Emulate_EEPROM_Write(_eep_index_RTC_AmPm, LOBYTE(sReadRTC.u32AmPm));
	FLASH_Lock();
	printf("record RTC data FINISH\r\n");	
	#endif
	
	if (report_flag)
	{
		// report by UART
		// clear log
	}

    printf("%s finish\r\n",__FUNCTION__);	
}

unsigned char Is_standby_mode_GPIO_high(void)
{
	// check GPIO , pressed
	uint8_t res = FALSE;

	res = (STANDBY_GPIO == FALSE) ? TRUE : FALSE ;
//	res = (STANDBY_GPIO == TRUE) ? TRUE : FALSE ;
	
	return res;
}


unsigned char Is_standby_mode_detect_true(void)
{
	/*
		is GPIO high : MCU normal mode
		is GPIO low : standby mode
	*/

	if (Is_standby_mode_GPIO_high())
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}	
}

unsigned char Is_SW_GPIO_high(void)
{
		// check GPIO , pressed
		uint8_t res = FALSE;
	
		res = (SW_GPIO == FALSE) ? TRUE : FALSE ;
	//	res = (SW_GPIO == TRUE) ? TRUE : FALSE ;
		
		return res;

}


void entry_normal_mode(void)
{
	set_flag(flag_wake_up_by_timer , TRUE);
}

void entry_standby_mode(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable RTC wake-up from SPD and DPD */
    CLK_ENABLE_RTCWK();

    /* Lock protected registers */
    SYS_LockReg();

	disable_RTC_tick();
    TIMER_DisableInt(TIMER1);
	WakeUpTimerFunction(CLK_PMUCTL_PDMSEL_DPD, CLK_PMUCTL_WKTMRIS_3277);

}

void flow_ctrl(void)
{
	/*
		if (standby mode GPIO high)
		{
			normal run
				if (SW GPIO high)
				{
					read RTC log and record
					report log 
					clear log
				}
		}
		
		if (standby mode GPIO low)
		{
			entry stand by
				wake up per 8 ms and check SW		
				if (SW GPIO high)
				{
					read RTC log and record
					report log 
					clear log
				}
		}

		
	*/

	if (Is_standby_mode_detect_true())
	{
		entry_normal_mode();

	}
	else
	{
		entry_standby_mode();
	}

	if (is_flag_set(flag_wake_up_by_timer))
	{
		if (Is_SW_GPIO_high())
		{
			// report RTC data
			RTC_report_log();
		}
		set_flag(flag_wake_up_by_timer , FALSE);
	}
	
}

//void GPF_IRQHandler(void)
//{
//    volatile uint32_t u32temp;

//    if(GPIO_GET_INT_FLAG(PF, BIT11))
//    {
//        GPIO_CLR_INT_FLAG(PF, BIT11);
//        printf("PF.11 INT occurred.\n");
//    }
//    else
//    {
//        /* Un-expected interrupt. Just clear all PC interrupts */
//        u32temp = PF->INTSRC;
//        PF->INTSRC = u32temp;
//        printf("Un-expected interrupts.\n");
//    }
//}


void Custom_GPIO_Init (void)
{
	// PF.11 , to emulate SW GPIO
    GPIO_SetMode(SW_GPIO_PORT, SW_GPIO_PIN_MASK, GPIO_MODE_INPUT);
    GPIO_EnableInt(SW_GPIO_PORT, SW_GPIO_PIN, GPIO_INT_RISING);
//    NVIC_EnableIRQ(GPF_IRQn);    
    GPIO_SET_DEBOUNCE_TIME(SW_GPIO_PORT, GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1024);
    GPIO_ENABLE_DEBOUNCE(SW_GPIO_PORT, SW_GPIO_PIN_MASK);    

	// PA2 , to emulate standby mode GPIO
    GPIO_SetMode(STANDBY_GPIO_PORT, STANDBY_GPIO_PIN_MASK, GPIO_MODE_INPUT);
    GPIO_EnableInt(STANDBY_GPIO_PORT, STANDBY_GPIO_PIN, GPIO_INT_RISING);
    GPIO_SET_DEBOUNCE_TIME(STANDBY_GPIO_PORT, GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1024);
    GPIO_ENABLE_DEBOUNCE(STANDBY_GPIO_PORT, STANDBY_GPIO_PIN_MASK);    

}

void RTC_IRQHandler(void)
{      
  
    /* To check if RTC tick interrupt occurred */
    if(RTC_GET_TICK_INT_FLAG(RTC) == 1)
    {
        /* Clear RTC tick interrupt flag */
        RTC_CLEAR_TICK_INT_FLAG(RTC);
        while((RTC_GET_TICK_INT_FLAG(RTC) == 1));
    }
}

void RTC_Init (void)
{
    S_RTC_TIME_DATA_T sInitTime;
	S_RTC_TIME_DATA_T recallTime;
	uint8_t msb = 0;
	uint8_t lsb = 0;

    /* Enable LXT clock if it is not enabled before */
    if( (CLK->PWRCTL & CLK_PWRCTL_LXTEN_Msk) == 0 )
    {
        CLK->PWRCTL |= CLK_PWRCTL_LXTEN_Msk;
        while((CLK->STATUS&CLK_STATUS_LXTSTB_Msk) == 0);        
    }     

    CLK_EnableModuleClock(RTC_MODULE);
    CLK_SetModuleClock(RTC_MODULE, RTC_LXTCTL_RTCCKSEL_LXT, (uint32_t)NULL);

    /* Initial RTC if it is not initialed before */    
    if(RTC->INIT != RTC_INIT_ACTIVE_Msk)
    {                              
        /* Open RTC */
        sInitTime.u32Year       = 2022;
        sInitTime.u32Month      = 1;
        sInitTime.u32Day        = 24;
        sInitTime.u32DayOfWeek  = RTC_MONDAY;
        sInitTime.u32Hour       = 0;
        sInitTime.u32Minute     = 0;
        sInitTime.u32Second     = 0;
        sInitTime.u32TimeScale  = RTC_CLOCK_24;
        RTC_Open(&sInitTime);
        printf("# Set RTC current date/time: 2022/01/24 00:00:00.\n\n");             

		FLASH_Unlock();
		Emulate_EEPROM_Write(_eep_index_RTC_is_record , TRUE);
		FLASH_Lock();
        
        /* Enable RTC tick interrupt and wake-up function will be enabled also */
        RTC_EnableInt(RTC_INTEN_TICKIEN_Msk);
        RTC_SetTickPeriod(RTC_TICK_1_SEC);           
    }
    else
    {
    	FLASH_Unlock();
		if (Emulate_EEPROM_Read(_eep_index_RTC_is_record) == TRUE)
		{
			msb = Emulate_EEPROM_Read(_eep_index_RTC_Year_MSB);
			lsb = Emulate_EEPROM_Read(_eep_index_RTC_Year_LSB);
			recallTime.u32Year       	= MAKEWORD(msb , lsb);		
			recallTime.u32Month      	= Emulate_EEPROM_Read(_eep_index_RTC_Month);
			recallTime.u32Day        	= Emulate_EEPROM_Read(_eep_index_RTC_Day);
			recallTime.u32DayOfWeek  	= Emulate_EEPROM_Read(_eep_index_RTC_DayOfWeek);			
			recallTime.u32Hour       	= Emulate_EEPROM_Read(_eep_index_RTC_Hour);
			recallTime.u32Minute     	= Emulate_EEPROM_Read(_eep_index_RTC_Minute);
			recallTime.u32Second     	= Emulate_EEPROM_Read(_eep_index_RTC_Second);
			recallTime.u32TimeScale     = Emulate_EEPROM_Read(_eep_index_RTC_TimeScale);			
			recallTime.u32AmPm  		= Emulate_EEPROM_Read(_eep_index_RTC_AmPm);
			
			//RTC_SetDateAndTime(&recallTime);
			RTC_Open(&recallTime);
			
			printf("%s : recall \r\n" , __FUNCTION__);

	        RTC_EnableInt(RTC_INTEN_TICKIEN_Msk);
	        RTC_SetTickPeriod(RTC_TICK_1_SEC);    				
		}
		FLASH_Lock();
    }
    
    RTC_CLEAR_TICK_INT_FLAG(RTC);

    /* Enable RTC NVIC */
    NVIC_EnableIRQ(RTC_IRQn);

}


void GPIO_Init (void)
{
    SYS->GPD_MFPL = (SYS->GPD_MFPL & ~(SYS_GPD_MFPL_PD2MFP_Msk)) | (SYS_GPD_MFPL_PD3MFP_Msk);
		
    GPIO_SetMode(PD, BIT2, GPIO_MODE_OUTPUT);

    GPIO_SetMode(PD, BIT3, GPIO_MODE_OUTPUT);	

	PD2 = 0;
	PD3 = 0;
}


void CheckPowerSource(void)
{
    uint32_t u32RegRstsrc;
    u32RegRstsrc = CLK_GetPMUWKSrc();

    printf("Power manager Power Manager Status 0x%x\n", u32RegRstsrc);

    if((u32RegRstsrc & CLK_PMUSTS_RTCWK_Msk) != 0)
        printf("Wake-up source is RTC.\n");
    if((u32RegRstsrc & CLK_PMUSTS_TMRWK_Msk) != 0)
    {
        printf("Wake-up source is Wake-up Timer.\n");
		set_flag(flag_wake_up_by_timer , TRUE);        
    }
    if((u32RegRstsrc & CLK_PMUSTS_PINWK_Msk) != 0)
        printf("Wake-up source is Wake-up Pin.\n");

    /* Clear all wake-up flag */
    CLK->PMUSTS |= CLK_PMUSTS_CLRWK_Msk;
}


void GpioPinSetting(void)
{
    /* Set function pin to GPIO mode */
    SYS->GPA_MFPH = 0;
    SYS->GPA_MFPL = 0;
    SYS->GPB_MFPH = 0;
    SYS->GPB_MFPL = 0;
    SYS->GPC_MFPH = 0;
    SYS->GPC_MFPL = 0;
    SYS->GPD_MFPH = 0;
    SYS->GPD_MFPL = 0;
    SYS->GPE_MFPH = 0;
    SYS->GPE_MFPL = 0;
    SYS->GPF_MFPH = 0;
    SYS->GPF_MFPL = 0x000000EE; //ICE pin
    SYS->GPG_MFPH = 0;
    SYS->GPG_MFPL = 0;
    SYS->GPH_MFPH = 0;
    SYS->GPH_MFPL = 0;

    /* Set all GPIOs are output mode */
    PA->MODE = 0x55555555;
    PB->MODE = 0x55555555;
    PC->MODE = 0x55555555;
    PD->MODE = 0x55555555;
    PE->MODE = 0x55555555;
    PF->MODE = 0x55555555;
    PG->MODE = 0x55555555;
    PH->MODE = 0x55555555;

    /* Set all GPIOs are output high */
    PA->DOUT = 0xFFFFFFFF;
    PB->DOUT = 0xFFFFFFFF;
    PC->DOUT = 0xFFFFFFFF;
    PD->DOUT = 0xFFFFFFFF;
    PE->DOUT = 0xFFFFFFFF;
    PF->DOUT = 0xFFFFFFFF;
    PG->DOUT = 0xFFFFFFFF;
    PH->DOUT = 0xFFFFFFFF;
}

void reduce_pwr_consumption(void)
{
    /* Set IO State and all IPs clock disable for power consumption */
    GpioPinSetting();

    CLK->APBCLK1 = 0x00000000;
    CLK->APBCLK0 = 0x00000000;

    /* ---------- Turn off RTC  -------- */
    CLK->APBCLK0 |= CLK_APBCLK0_RTCCKEN_Msk;
    RTC->INTEN = 0;
    CLK->APBCLK0 &= ~CLK_APBCLK0_RTCCKEN_Msk;

}


void TMR1_IRQHandler(void)
{
//	static uint32_t LOG = 0;

	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
//        	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
			PD2 ^= 1;
		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}


void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res == 'x' || res == 'X')
	{
		NVIC_SystemReset();
	}

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		switch(res)
		{
			case '1':
				break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
				NVIC_SystemReset();		
				break;
		}
	}
}

void UART0_IRQHandler(void)
{

    if(UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
            UARTx_Process();
        }
    }

    if(UART0->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        UART_ClearIntFlag(UART0, (UART_INTSTS_RLSINT_Msk| UART_INTSTS_BUFERRINT_Msk));
    }	
}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
    NVIC_EnableIRQ(UART0_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());	
	#endif	

}



void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set PF multi-function pins for XT1_OUT(PF.2) and XT1_IN(PF.3) */
    SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF2MFP_Msk)) | SYS_GPF_MFPL_PF2MFP_XT1_OUT;
    SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF3MFP_Msk)) | SYS_GPF_MFPL_PF3MFP_XT1_IN;

    /* Set PF multi-function pins for X32_OUT(PF.4) and X32_IN(PF.5) */
    SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF4MFP_Msk)) | SYS_GPF_MFPL_PF4MFP_X32_OUT;
    SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF5MFP_Msk)) | SYS_GPF_MFPL_PF5MFP_X32_IN;

    reduce_pwr_consumption();

    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);	

    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);	

    CLK_SetCoreClock(FREQ_96MHZ);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL, CLK_CLKDIV0_HCLK(1));

    /* Enable UART clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART clock source from HIRC */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL2_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);

    SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA6MFP_Msk | SYS_GPA_MFPL_PA7MFP_Msk)) |
                    (SYS_GPA_MFPL_PA6MFP_UART0_RXD | SYS_GPA_MFPL_PA7MFP_UART0_TXD);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}


/*
 * This is a template project for M251 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */
int main()
{
    SYS_Init();

	UART0_Init();
	GPIO_Init();
	TIMER1_Init();

	DataFlashInit();
	CheckPowerSource();
	Custom_GPIO_Init();
	RTC_Init();

    /* Got no where to go, just loop forever */
    while(1)
    {
		flow_ctrl();

    }
}


/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
