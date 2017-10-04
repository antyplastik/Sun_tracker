/*
 * adc.c
 *
 *  Created on: Mar 13, 2016
 *      Author: Kamil
 */

/*
 *
 * CZTERY POTRZEBNE KANALY ADC DO POMIARU MOCY DZIALAJA.
 * Pozostaje kalibracja i podlaczenie do reszty programu.
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "adc.h"

#include "stm32f30x.h"
#include "stm32f3_discovery.h"
#include "stm32f30x_it.h"
#include "stm32f30x_adc.h"

unsigned int  ADC1ConvertedValue = 0, calibration_value = 0, ADC1ConvertedVoltage = 0;
unsigned int TimingDelay = 0;

int g_ADCTest = 0;


volatile unsigned int g_ADC1_Measure;
volatile char g_ADC_PV_Measure_str[12] = {"0\0"};

// kanaly ADC1 zczytywane po kolei
uint32_t g_ADC_PV_value [4]; // g_ADC_PV_value [4] = {PV_1_V, PV_1_I, PV_2_V, PV_2_I}
//int g_ADC_PV_table_index = 0; // ustawione poczatkowo na numer 1-ego kanalu
uint32_t g_ADC_PV_value_conv [ADC1_DMA_CHANNELS];

int g_ADC1_measure_counter = 0;

// Zakres pomiarowy ADC1
int g_ADC_PV_CH2_min = 0;	// dla CH2
int g_ADC_PV_CH2_max = 10;
int g_ADC_PV_CH3_min;	// dla CH3
int g_ADC_PV_CH3_max;
int g_ADC_PV_CH4_min = 0;	// dla CH4
int g_ADC_PV_CH4_max = 10;
int g_ADC_PV_CH5_min;	// dla CH5
int g_ADC_PV_CH5_max;

// Wspolczynnik podzialu napiecia w dzielniku		<= trzeba skalibrowac recznie
int g_ADC_voltage_divide_ratio_CH2 = 1;
int g_ADC_voltage_divide_ratio_CH4 = 1;

// R dla pomiaru pradu								<= trzeba skalibrowac recznie
int g_ADC_Resistance_CH3 = 1;
int g_ADC_Resistance_CH5 = 1;

// Tabele korekcyjne bledu odczytu (w dzialkach) dla 1/4, 2/4, 3/4 i 4/4 zakresu (docelowo zamiast zmiennych maja byc tablice)
int g_ADC_cal_corr_CH2;
int g_ADC_cal_corr_CH3;
int g_ADC_cal_corr_CH4;
int g_ADC_cal_corr_CH5;

int g_ADC_PV_cal_corr = 26;



void ADC1_PV_Init()
{
	// wartosci domyslne
	//uint8_t ADCstatus = 1; // 1" wlacza przetwornik ADC
	//unsigned int adc1timperiod = 65535;
	//unsigned int adc1timprescaler = 1;

	//uint32_t *ADC_V_Measure = &g_ADC1_Measure;
	//*ADC_V_Measure = 0;

	ADC1_GPIO_Init();
	ADC1_Config();
	//Cyclic_Measure_TIM(adc1timperiod, adc1timprescaler);
	//ADC_V_Measure();


}

void ADC1_Config()
{
	/*
	 * Funkcja konfigurujaca ADC
	 */
	int i;
	int t = SystemCoreClock / 100; //delay 10ms

	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div2); // wlacza dzielnik zegara ADC
	RCC_AHBPeriphClockCmd(ADC_V_Rcc, ENABLE); // wlacza zegar ADC
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE); // wlacza zegar DMA1


	DMA_InitTypeDef	DMA_InitStructure;

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &ADC_PV->DR;				//specify the base address of the peripheral (ADC1) to connect to the DMA1 channel 1
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &g_ADC_PV_value[0]; 						//specify the base address of the result array
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 4;											//=2 because we are using 2 channels and each channel is stored in an array element
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;							//automatically increase the element number for each channel conversion
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;									//after all the channels are read by the DMA, start again // JEDNA KOLEJKA
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(ADC_PV_DMA1_CH1, &DMA_InitStructure);

	DMA_Cmd(ADC_PV_DMA1_CH1, ENABLE);

	/*
	 * wbudowana procedura autokalibracji wg wbudowanej referencji
	 */
//---------------------------------------------------

	ADC_VoltageRegulatorCmd(ADC_PV, ENABLE); // wlacza referencje
	//Delay(10);
	for (i=0 ; i>=t; i++);

	ADC_SelectCalibrationMode(ADC_PV, ADC_CalibrationMode_Single);
	ADC_StartCalibration(ADC_PV);

	while(ADC_GetCalibrationStatus(ADC_PV) != RESET );
	calibration_value = ADC_GetCalibrationValue(ADC_PV);
//---------------------------------------------------

	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = 10;
	ADC_CommonInit(ADC_PV, &ADC_CommonInitStructure);

	ADC_InitTypeDef       ADC_InitStructure;
	ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
	ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;
	ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Enable; // zmienione na probe
	ADC_InitStructure.ADC_NbrOfRegChannel = 4;
	ADC_Init(ADC_PV, &ADC_InitStructure);

	//ADC_StructInit(&ADC_InitStructure); //struktura domyslna taka jak powyzej

	//void ADC_RegularChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime);

	ADC_RegularChannelConfig(ADC_PV, PV_1_V_ADC_ch, 1, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC_PV, PV_1_I_ADC_ch, 2, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC_PV, PV_2_V_ADC_ch, 3, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC_PV, PV_2_I_ADC_ch, 4, ADC_SampleTime_7Cycles5);


	ADC_DMAConfig(ADC_PV, 2); // praca DMA w kolo. stad 2

	ADC_DMACmd(ADC_PV, ENABLE);

	ADC_Cmd(ADC_PV, ENABLE);

/*
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
*/


	while(!ADC_GetFlagStatus(ADC_PV, ADC_FLAG_RDY)); // jezeli pojawi sie flaga to wychodzi z petli

	ADC_StartConversion(ADC_PV); // nalezy odznaczyc dla continouse mode
}

void ADC1_GPIO_Init()
{
	//RCC_AHBPeriphClockCmd(ADC_V_GPIO_Rcc, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);

/*

	// Configure ADC Channel7 as analog input
	GPIO_InitStructure.GPIO_Pin = ADC_V_Pin; // kanal 7 na GPIOC jako kanal dodatkowy (poczatkowo skonfigurowany do nauki)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ADC_V_GPIO, &GPIO_InitStructure);
*/

	GPIO_InitTypeDef      GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = PV_1_V_GPIO | PV_1_I_GPIO | PV_2_V_GPIO; // kanaly 2,3,4 na GPIOA
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ADC_PV_GPIO_PA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PV_2_I_GPIO; // kanal 5 na GPIOF
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(ADC_PV_GPIO_PF, &GPIO_InitStructure);

}

void Cyclic_Measure_TIM(uint32_t adc1timperiod, uint32_t adc1timprescaler)
{
	/*
	 * zainicjowac timer
	 */

	RCC_APB2PeriphClockCmd(ADC_V_TIM_RCC, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

	TIM_TimeBaseInitStruct.TIM_Period = adc1timperiod;
	TIM_TimeBaseInitStruct.TIM_Prescaler = adc1timprescaler;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(ADC_V_TIM, &TIM_TimeBaseInitStruct);

	TIM_ClearFlag(ADC_V_TIM, TIM_FLAG_Update); // czysci flage aktualizacji TIM2
	TIM_ITConfig(ADC_V_TIM, TIM_IT_Update, ENABLE); //wlacza przerwanie aktualizacji TIM2
	TIM_Cmd(ADC_V_TIM, ENABLE); //wlacza timer
/*
	NVIC_InitTypeDef	NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = ADC_V_TIM_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
*/

}


void Delay(uint32_t nTime)
{
	TimingDelay = nTime;

	while(TimingDelay != 0);
}

void ADC_Get_Values()
{

	ADC_PV_Measure();
	ADC1_to_USART();
}


void ADC_Test()
{
	ADC_Get_Values();
}


void ADC_V_Measure()
{
	/*
	 * Funkcja realizuje pomiar ciagly z usrednianiem wykonywany n razy przy kazdym jej wykonaniu.
	 * Nadpisuje zmienna globalna g_ADC1_Measure, a takze wysyla string z wartoscia pomiaru pomiaru przez interfej USART.
	 */

	//Usrednianie wyniku

	unsigned int measure_count = 0;
	unsigned int measure_count_stop = 3;
	volatile unsigned int measure_table [measure_count_stop];
	unsigned int measure_temp = 0;
	unsigned int measure_mean = 0;

	/* Test EOC flag */
	//while(ADC_GetFlagStatus(ADC_PV, ADC_FLAG_EOC) == RESET);

		for (measure_count; measure_count <= measure_count_stop; measure_count++)
			{
				//ADC_V_Measure =	&g_ADC1_Measure;


				/* Get ADC1 converted data */
				ADC1ConvertedValue =ADC_GetConversionValue(ADC_PV);

				/* Compute the voltage */
				ADC1ConvertedVoltage = (ADC1ConvertedValue *3000)/0xFFF;


				if (ADC1ConvertedVoltage > g_ADC_PV_cal_corr && ADC1ConvertedVoltage >= 0)
					{
						if (g_ADC_PV_cal_corr >= 0)
							g_ADC1_Measure = ADC1ConvertedVoltage - g_ADC_PV_cal_corr;

						else if (g_ADC_PV_cal_corr < 0)
							g_ADC1_Measure = ADC1ConvertedVoltage + g_ADC_PV_cal_corr;
					}


				if(ADC1ConvertedVoltage <= g_ADC_PV_cal_corr)
					{
						g_ADC1_Measure = ADC1ConvertedVoltage;
					}


				measure_table [measure_count] =  g_ADC1_Measure;

				measure_temp = measure_temp + measure_table[measure_count];


				if (measure_count == measure_count_stop)
					{
						/*
							for (measure_count; measure_count == 0; measure_count--)
							measure_temp = measure_temp + measure_table[measure_count];

							measure_mean = measure_temp / (measure_count_stop +1);

						 */
						measure_mean = measure_temp / (measure_count_stop+1);


						g_ADC1_Measure = measure_mean;

					}

			}

	sprintf((char *)g_ADC_PV_Measure_str, "Wartosc odczytana z przetwornika ADC1: %d mV \r\n",  g_ADC1_Measure);

	USART_Tx(g_ADC_PV_Measure_str);


}


/*
 * Projekt funkcji do wywolywania funkcji ADC_V_Measure() po przerwaniu gdy w reejstrze ADC pojawila sie kolejna wartosc
void ADC1_2_IRQHandler()
{
	while(ADC_GetFlagStatus(ADC_PV, ADC_FLAG_EOC) == RESET)
		ADC_V_Measure();
}
*/

void ADC_PV_Measure()
{
	/*
	 * Funkcja odczytuje tablice, ktora zapelnil DMA1, przetwarza na wartosci skuteczne, a nastepnie podmienia poszczegolne wartosci w tablicy;
	 */

	/*
	 * ROZPISKA KANALOW ADC1:
	 *
	 * PV_1_V_ADC_ch	ADC_Channel_2	g_ADC_PV_table_index = 0
	 * PV_1_I_ADC_ch	ADC_Channel_3	g_ADC_PV_table_index = 1
	 * PV_2_V_ADC_ch	ADC_Channel_4	g_ADC_PV_table_index = 2
	 * PV_2_I_ADC_ch	ADC_Channel_5	g_ADC_PV_table_index = 3
	 */

	/*
	 * 	POMIAR ======================================= Sam mechanizm funcji dziala, ale tablica jest niewypelniona!!!
	 */

	// zmienne tymczasowe
	uint32_t tmp_ADCreadvalue = 0;
	uint32_t tmp_ADCconvertedvalue = 0;
	//uint32_t tmp_ADC_V_value = 0;
	//uint32_t tmp_ADC_I_value = 0;
	uint32_t tmp_ADC_PV_table_index = 0;

	for (tmp_ADC_PV_table_index = 0; tmp_ADC_PV_table_index <= 3; tmp_ADC_PV_table_index++)
		g_ADC_PV_value_conv [tmp_ADC_PV_table_index] = g_ADC_PV_value [tmp_ADC_PV_table_index]; // kopiowanie tabl. wypeln. przez DMA do tabl gdzie wart skonwertowane


	for (tmp_ADC_PV_table_index = 0; tmp_ADC_PV_table_index <= 3; tmp_ADC_PV_table_index++)
	{

		if (tmp_ADC_PV_table_index == 0 || tmp_ADC_PV_table_index == 2) // napiecie
		{
			tmp_ADCreadvalue = g_ADC_PV_value[tmp_ADC_PV_table_index];
			tmp_ADCconvertedvalue = (tmp_ADCreadvalue * 3300)/0xFFF;

			/*// warunki niepotrzebne
			if(tmp_ADC_PV_table_index == 0)
				tmp_ADC_V_value = tmp_ADCconvertedvalue / g_ADC_voltage_divide_ratio_CH2;

			if(tmp_ADC_PV_table_index == 2)
				tmp_ADC_V_value = tmp_ADCconvertedvalue / g_ADC_voltage_divide_ratio_CH4;
			*/

			//g_ADC_PV_value_conv [tmp_ADC_PV_table_index] = tmp_ADCconvertedvalue; // <== to jebie opcje!!!!

		}


		if (tmp_ADC_PV_table_index == 1 || tmp_ADC_PV_table_index == 3) // prad wyrazony w mA
		{
			tmp_ADCreadvalue = g_ADC_PV_value[tmp_ADC_PV_table_index];
			tmp_ADCconvertedvalue = (tmp_ADCreadvalue * 3300)/0xFFF;

			/*// warunki niepotrzebne
			if (tmp_ADC_PV_table_index == 1)
				tmp_ADC_I_value = tmp_ADCconvertedvalue / g_ADC_Resistance_CH3;

			if (tmp_ADC_PV_table_index == 3)
				tmp_ADC_I_value = tmp_ADCconvertedvalue / g_ADC_Resistance_CH5;
			*/

			//g_ADC_PV_value_conv [tmp_ADC_PV_table_index] = tmp_ADCconvertedvalue;

		}
		g_ADC_PV_value_conv [tmp_ADC_PV_table_index] = tmp_ADCconvertedvalue;

	}

	g_ADC1_measure_counter++; //licznik pomiarow

	/*
	 * 	PRZEKAZANIE WYNIKU
	 */

}


void ADC1_to_USART()
{

	/*
	 * Funkcja  dziala
	 */

	uint32_t tmp_ADC_PV_table_index = 0;

	for (tmp_ADC_PV_table_index = 0; tmp_ADC_PV_table_index <= 3; tmp_ADC_PV_table_index++)
	{

		if (tmp_ADC_PV_table_index == 0)
			sprintf((char *)g_ADC_PV_Measure_str, "V1: %d mV", g_ADC_PV_value_conv[tmp_ADC_PV_table_index]);

		if (tmp_ADC_PV_table_index == 1)
			sprintf((char *)g_ADC_PV_Measure_str, "I1: %d mA", g_ADC_PV_value_conv[tmp_ADC_PV_table_index]);

		if (tmp_ADC_PV_table_index == 2)
			sprintf((char *)g_ADC_PV_Measure_str, "V2: %d mV", g_ADC_PV_value_conv[tmp_ADC_PV_table_index]);

		if (tmp_ADC_PV_table_index == 3)
			sprintf((char *)g_ADC_PV_Measure_str, "I2: %d mA", g_ADC_PV_value_conv[tmp_ADC_PV_table_index]);


		USART_Tx(g_ADC_PV_Measure_str);


		if (tmp_ADC_PV_table_index < 3)
			USART_Tx("   ");

		else if (tmp_ADC_PV_table_index == 3)
			USART_Tx("\n\r");


/*
 * Znaki:
 *  \r wyswietla dane w kolumnie (pierwszy znak string wedruje do poczatku transmitowanej linijki)
 *  \n zaczyna od nowej linii
 */

	}
}


void DMA1_Channel1_IRQHandler()
{



}

void ADC1_2_IRQHandler()
{



}

void TIM1_BRK_TIM15_IRQHandler()
{
	/* Przerwania z licznika nie dzialaja */

	if(TIM_GetITStatus(ADC_V_TIM, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(ADC_V_TIM, TIM_IT_Update);
		ADC_V_Measure();
	}
}
