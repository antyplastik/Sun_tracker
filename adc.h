/*
 * adc.h
 *
 *  Created on: Mar 13, 2016
 *      Author: Kamil
 */

#ifndef ADC_H_
#define ADC_H_

#define PV_1_V_GPIO		GPIO_Pin_1	//	PA1
#define PV_1_I_GPIO		GPIO_Pin_2	//	PA2
#define PV_2_V_GPIO		GPIO_Pin_3	//	PA3
#define PV_2_I_GPIO 	GPIO_Pin_4	//	PF4

#define PV_1_V_ADC_ch	ADC_Channel_2
#define PV_1_I_ADC_ch	ADC_Channel_3
#define PV_2_V_ADC_ch	ADC_Channel_4
#define PV_2_I_ADC_ch	ADC_Channel_5

#define SerV_1_ADC_ch	ADC_Channel_6	//	PC0	wlaczyc
#define SerV_2_ADC_ch	ADC_Channel_8	//	PC2	wlaczyc

#define ADC_PV			ADC1

#define ADC_PV_GPIO_PA		GPIOA
#define ADC_PV_GPIO_PF		GPIOF
#define ADC_SerV_GPIO_PC	GPIOC

#define ADC_PV_RCC_PA		RCC_AHBPeriph_GPIOA
#define ADC_PV_RCC_PF		RCC_AHBPeriph_GPIOF
#define ADC_SerV_RCC_PC		RCC_AHBPeriph_GPIOC	// wlaczyc

#define ADC_PV_DMA1_CH1		DMA1_Channel1
#define ADC1_DMA_CHANNELS	4 // zdefiniowana ilosc kanalow ktore sa pobierane z tabeli ktora pobiera DMA

#define ADC_V_GPIO		GPIOC
#define ADC_V_Pin		GPIO_Pin_1
//#define ADC_V_Pin		GPIO_Pin_0
#define ADC_V_Rcc		RCC_AHBPeriph_ADC12 //tyczy sie zegara do ktorego podlaczone sa ADC1 i ADC2
#define ADC_V_GPIO_Rcc	RCC_AHBPeriph_GPIOC

//#define ADC_V_GPIO_Rcc	RCC_AHBPeriph_GPIOA

#define ADC_V_TIM		TIM15
#define ADC_V_TIM_RCC	RCC_APB2Periph_TIM15
#define ADC_V_TIM_IRQ	TIM1_BRK_TIM15_IRQn

void ADC1_PV_Init();
void ADC1_Config();
void ADC1_GPIO_Init();
void Cyclic_Measure_TIM(uint32_t adc1timperiod, uint32_t adc1timprescaler);
void ADC_Get_Values();
void ADC_Test();
void ADC_PV_Measure();
void ADC1_to_USART();
void Delay(uint32_t nTime);



#endif /* ADC_H_ */
