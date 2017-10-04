/*
 * rtc.c
 *
 *  Created on: Sep 29, 2016
 *      Author: Kamil
 */


#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "rtc.h"

#include "stm32f30x.h"
#include "stm32f3_discovery.h"
#include "stm32f30x_it.h"
#include "stm32f30x_pwr.h"

static unsigned char g_RTC_showtime [50] = {"0\0"};
static unsigned char g_RTC_showdate [50] = {"0\0"};

uint32_t g_RTC_Year = 0;
uint32_t g_RTC_month = 0;
uint32_t g_RTC_day = 0;
uint32_t g_RTC_hour = 0;
uint32_t g_RTC_minute = 0;
uint32_t g_RTC_second = 0;

//uint32_t g_RTC_RDY;
/*
uint8_t g_RTC_ALARM_Year;
uint8_t g_RTC_ALARM_month;
uint8_t g_RTC_ALARM_day;
uint8_t g_RTC_ALARM_hour;
uint8_t g_RTC_ALARM_minute;
uint8_t g_RTC_ALARM_second;
*/

RTC_DateTypeDef RTC_DateStructure;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_AlarmTypeDef RTC_AlarmStructure;


void RTC_PV_Init()
{
	//STM_EVAL_LEDInit(LED3 | LED6);
	RTC_PV_Config();
	//RTC_Time_to_USART();
	//STM_EVAL_LEDOn(LED6);

	//RTC_Set_Time(22,30,0);
	//RTC_Set_Date(16,10,6);


}

void RTC_PV_Config()
{

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset RTC Domain */
	RCC_BackupResetCmd(ENABLE);
	RCC_BackupResetCmd(DISABLE);



	/* The RTC Clock may varies due to LSI frequency dispersion */
	/* Enable the LSI OSC */
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);


	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro();


	RTC_InitTypeDef RTC_InitStructure;

	NVIC_InitTypeDef NVIC_InitStructure;

	/* RTC prescaler configuration */
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_InitStructure.RTC_AsynchPrediv = 88;
	RTC_InitStructure.RTC_SynchPrediv = 470;
	RTC_Init(&RTC_InitStructure);



	/* Enable RTC Alarm A Interrupt */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);		// mozna zaprogramowac takze alarm B
	RTC_ITConfig(RTC_IT_ALRB, ENABLE);


	/* Enable the alarm */
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
	RTC_AlarmCmd(RTC_Alarm_B, ENABLE);

	RTC_ClearFlag(RTC_FLAG_ALRAF);
	RTC_ClearFlag(RTC_FLAG_ALRBF);

	/* Enable the RTC Alarm Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


}


void RTC_Set_Time(uint32_t hour, uint32_t minute, uint32_t second)
{

	/* Set the time to 01h 00mn 00s AM */
	//RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
	g_RTC_hour = RTC_TimeStructure.RTC_Hours   = hour;
	g_RTC_minute = RTC_TimeStructure.RTC_Minutes = minute;
	g_RTC_second = RTC_TimeStructure.RTC_Seconds = second;
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

}


void RTC_Set_Date(uint32_t Year, uint32_t month, uint32_t day)
{

	g_RTC_Year = Year;
	g_RTC_month = month;
	g_RTC_day = day;

	Year = Year - 2000;

	/* Set the date: Wednesday August 15th 2012 */
	RTC_DateStructure.RTC_Year = Year; //korekcja poniewaz licza sie ostatnie 2 cyfry roku
	RTC_DateStructure.RTC_Month = month;
	RTC_DateStructure.RTC_Date = day;
	//RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Tuesday;
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);

}

/*
void RTC_Set_Alarm_A(uint32_t hour, uint32_t minute, uint32_t second)
{
	// Set the alarm 01h:00min:04s
	//RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
	g_RTC_ALARM_hour = RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = hour;
	g_RTC_ALARM_minute = RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = minute;
	g_RTC_ALARM_second = RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = second;
	//RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
	//RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	// Alarm mask hour, min and second:default Alarm generation each 1s
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_All;
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);


}
*/

void RTC_Time_to_USART()
{

    /* Get the RTC current Date */
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

    /* Get the RTC current Time */
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    USART_Tx("RTC: ");
    sprintf((char*)g_RTC_showdate,"%d-%d-%d", 2000 + RTC_DateStructure.RTC_Year, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date);
    USART_Tx(g_RTC_showdate);
    USART_Tx(" ");

    sprintf((char*)g_RTC_showtime,"%d:%d:%d",RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
    USART_Tx(g_RTC_showtime);
    USART_Tx("\n\r");

}


void RTC_Time_to_g_variables()
{
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

	g_RTC_Year = (uint32_t) RTC_DateStructure.RTC_Year;
	g_RTC_Year = g_RTC_Year + 2000;
	g_RTC_month = (uint32_t) RTC_DateStructure.RTC_Month;
	g_RTC_day = (uint32_t) RTC_DateStructure.RTC_Date;
	g_RTC_hour = (uint32_t) RTC_TimeStructure.RTC_Hours;
	g_RTC_minute = (uint32_t) RTC_TimeStructure.RTC_Minutes;
	g_RTC_second = (uint32_t) RTC_TimeStructure.RTC_Seconds;

}


void TAMPER_STAMP_IRQHandler()
{

}
