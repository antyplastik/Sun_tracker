/*
 * interrupts.c
 *
 *  Created on: Mar 8, 2015
 *      Author: Kamil
 */


#include "interrupts.h"
#include "main.h"

#include "stm32f30x_gpio.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_misc.h"




void Button_Init_IRQ()
{
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG ,ENABLE);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	EXTI_InitStruct.EXTI_Line = EXTI_Line0;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init( &EXTI_InitStruct);

	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init( &NVIC_InitStruct);

	//NVIC_EnableIRQ(EXTI0_IRQn); //niby wlaczone przerwanie EXTI0 w NVIC
}




void EXTI0_IRQHandler()
{
	//uint32_t *p_ADC1_Measure = extern g_ADC1_Measure;
	if (EXTI_GetITStatus(EXTI_Line0) != RESET)
	{

		uint16_t i=0;
		for (i; i<10000; i++); //filtruje odbicia z przycisku

		ChangeStateLED();
		//Data = 'a';
		//USART_SendData(USART1,Data);
		//i=0;
		//for (i; i<10000; i++);
		//USART_Tx("Czesc!\r\n");
		//USART_Tx(); // wartosc z pomiaru ADC1
		//ADC_V_Measure();
		//ADC1_2_IRQHandler();

		ADC_Get_Values();
		RTC_Time_to_USART();

		EXTI_ClearITPendingBit(EXTI_Line0);
		//i=0;

	}
}




