/*
 * rtc.h
 *
 *  Created on: Sep 29, 2016
 *      Author: Kamil
 */


#ifndef RTC_H_
#define RTC_H_

typedef enum
{
	RTC_Month_January	= 1,
	RTC_Month_February	= 2,
	RTC_Month_March		= 3,
	RTC_Month_April		= 4,
	RTC_Month_May		= 5,
	RTC_Month_June		= 6,
	RTC_Month_July		= 7,
	RTC_Month_August	= 8,
	RTC_Month_September	= 9,
	RTC_Month_October	= 10,
	RTC_Month_November	= 11,
	RTC_Month_December	= 12

}RTC_month_TypeDef;

typedef enum
{
	RTC_Weekday_Monday		= 1,
	RTC_Weekday_Tuesday		= 2,
	RTC_Weekday_Wednesday	= 3,
	RTC_Weekday_Thursday	= 4,
	RTC_Weekday_Friday		= 5,
	RTC_Weekday_Saturday	= 6,
	RTC_Weekday_Sunday		= 7

}RTC_weekday_TypeDef;


void RTC_PV_Init();
void RTC_PV_Config();

void RTC_Set_Time(uint32_t hour, uint32_t minute, uint32_t second);
void RTC_Set_Date(uint32_t Year, uint32_t month, uint32_t day);
void RTC_Set_Alarm_A(uint32_t hour, uint32_t minute, uint32_t second);


void RTC_Time_to_USART();
void RTC_Time_to_g_variables();


#endif /* RTC_H_ */
