/*
 * servo.c
 *
 *  Created on: Jan 10, 2016
 *      Author: Kamil
 */
#include <stddef.h>
#include <stdint.h>

#include "main.h"
#include "servo.h"

#include "stm32f30x_gpio.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_tim.h"
#include "stm32f30x_tim.h"


float servo_H;// = 90; //podlaczyc bo wymagana jest zmienna zewnetrzna do obliczen
float servo_V;// = 45;


void Servo_Init() // main servo function
{
	//--------- domyslna konfiguracja pwm ----------
	int32_t pwmprescaler = 71; //tim taktuje z czestotliwoscia zegara glownego
	int32_t pwmperiod = 20000; // 72MHz / (5000-1) = 14402,88058 <- 5 kHz to czestotliwosc taktowania PWM

// Konfiguracja domyslna polozenia walu
	//int32_t servo_H=180;
	//int32_t servo_V=45;

	//PWM_LED_Timer_Init();

	//LED_Warning_Init();

	Servo_IO_Init();
	PWM_Servo_Timer_Init(pwmprescaler, pwmperiod);
	//Servo_Ctrl(servo_H, servo_V);
	//USART_Tx("Uruchomiona obsluga serwomechanizmow \r\n");

}

void Servo_Ctrl(int32_t servo_H, int32_t servo_V)
{
	/*
	 * V	90st 	- 1,40ms	(0st)
	 * 		180st 	- 2,24ms	(90st)
	 * H	0st 	- 2,32ms
	 * 		90st 	- 1,44ms
	 * 		180st 	- 0,64ms
	 */


	int32_t MAX_pulse_lenght_H = 2350; // skrajne prawo = 180 st
	int32_t MIN_pulse_lenght_H = 650; // skrajne lewo = 0 st
	int32_t MAX_pulse_lenght_V = 2250; // polozenie 180 st (PV rownolegle do podloza)
	int32_t MIN_pulse_lenght_V = 1400;  // polozenie 90 st (PV prostopadle do podloza)

	servo_H = (MAX_pulse_lenght_H - MIN_pulse_lenght_H) * servo_H / 180 + MIN_pulse_lenght_H;
	servo_V = (MAX_pulse_lenght_V - MIN_pulse_lenght_V) * servo_V / 90 + MIN_pulse_lenght_V;

	Servo_V_PWM(servo_V);
	Servo_H_PWM(servo_H);

}

void Servo_V_PWM(int32_t servo_V)
{
	TIM_SetCompare1(Servo_TIM, servo_V);
}

void Servo_H_PWM(int32_t servo_H)
{
	TIM_SetCompare2(Servo_TIM, servo_H);
}

void PWM_Servo_Timer_Init(int32_t pwmprescaler, int32_t pwmperiod)
{
	//Funkcja inicjuje PWM dla serw i LED

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

	TIM_TimeBaseInitStruct.TIM_Period = pwmperiod;
	TIM_TimeBaseInitStruct.TIM_Prescaler = pwmprescaler;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseInitStruct);

	TIM_Cmd(TIM8, ENABLE); //wlacza licznik
	TIM_CtrlPWMOutputs(Servo_TIM, ENABLE);

/*
	TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE); //wlacza przerwania z licznika

	NVIC_InitTypeDef NVIC_TimmerInterruptInitStruct;

	NVIC_TimmerInterruptInitStruct.NVIC_IRQChannel = TIM8_UP_IRQn;
	NVIC_TimmerInterruptInitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_TimmerInterruptInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_TimmerInterruptInitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_TimmerInterruptInitStruct);
*/

	/*
	 * Kanaly PWM:
	 * - PC6	TIM_CH1	(Servo V)
	 * - PC7	TIM_CH2 (Servo H)
	 */

	TIM_OCInitTypeDef TIM_OCStruct;

	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCStruct.TIM_OCIdleState = TIM_OCIdleState_Set;
	//TIM_OCStruct.TIM_Pulse = 1700;

	TIM_OC1Init(Servo_TIM, &TIM_OCStruct);
	TIM_OC2Init(Servo_TIM, &TIM_OCStruct);


}


void Servo_IO_Init()
{
	RCC_AHBPeriphClockCmd(ServoPortRCC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.GPIO_Mode = 	GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = 	GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = 		Servo_H_Out | Servo_V_Out;
	GPIO_InitStruct.GPIO_PuPd = 	GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = 	GPIO_Speed_Level_2;

	GPIO_Init(ServoPort, &GPIO_InitStruct);

	GPIO_PinAFConfig(ServoPort,Servo_H_AF,GPIO_AF_4);
	GPIO_PinAFConfig(ServoPort,Servo_V_AF,GPIO_AF_4);

}

/*
void LED_Warning_Init()
{
//	LED_Warning_H_Init();
//	LED_Warning_V_Init();
}

void LED_Warning_H_Init()
{

}

void LED_Warning_V_Init()
{

}

void ChangeStateWarningLED()
{

	if(GPIO_ReadOutputData(LEDPWMPort) & LED_Warning_H_PWM)
		GPIO_WriteBit(LEDPWMPort, LED_Warning_H_PWM, Bit_RESET);
	else
		GPIO_WriteBit(LEDPWMPort, LED_Warning_H_PWM, Bit_SET);
}

*/
/*
void TIM8_IRQHandler()
{
	if (TIM_GetITStatus(TIM8, TIM_IT_Update) == SET)
		ChangeStateWarningLED();
}
*/
